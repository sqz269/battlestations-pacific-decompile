#pragma once
#include "bsp/native_render_context.hpp"
#include "bsp/native_render_zero_frame.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>

namespace bsp {
class NativeString;
class NativeMeshRetentionState;
class NativePostEffectOwner;
class NativePostEffectReference;
class NativePostEffect20Owner;
class NativePostEffect20Reference;
class NativePostEffectConstructionBlock;
class NativePostEffect20ConstructionBlock;
struct NativePostEffectConstructionContext;
struct NativePostEffect20ConstructionContext;
struct NativePostEffect20ArgumentView;
struct NativePostEffectOrdinaryLifetimeBinding;
class NativeRenderObservedZeroAccess;
class NativePostEffectParentFrame;
class NativeMaterialFrame;
struct NativeMaterialStorage;
struct NativeMaterialFactoryAcquired;
struct NativeMaterialEffectCacheContext;
class NativePostEffect20MaterialObservedZeroIssuer;
class NativePostEffect24MaterialObservedZeroIssuer;

// Source metadata over the actual two parent producers. The real caller must
// retain this Domain, its providers and the SAME nonmoving Blocks through all
// Native/Source return, health, observation and catch fences. This does not
// select the Mission/upper route, authenticate Original origin or mint credit.
class NativePostEffectParentDomain final {
public:
    NativePostEffectParentDomain(NativePostEffect20ConstructionContext&,
        NativePostEffectConstructionContext&,
        const NativePostEffectOrdinaryLifetimeBinding&,
        NativeRenderActualOwnerRegistry&, NativeMeshRetentionState&);
    ~NativePostEffectParentDomain(); // metadata only; no invented Native cleanup
    NativePostEffectParentDomain(const NativePostEffectParentDomain&) = delete;
    NativePostEffectParentDomain& operator=(const NativePostEffectParentDomain&) = delete;

    // The Domain owns these stable Frames. Preparation is before BF681B and
    // takes a concrete cold Block, never an already-born raw pointer or permit.
    NativePostEffectParentFrame& prepare20(NativePostEffect20ConstructionBlock&);
    NativePostEffectParentFrame& prepare24(NativePostEffectConstructionBlock&);
    bool requires_process_retention() const noexcept; // failure/unknown only
    bool has_retained_records() const noexcept;      // expected live is separate
    void close_after_host_quiescence(); // a real caller fence, not a counter test
private:
    friend class NativePostEffectParentFrame;
    // Pure all-record validation, including each selected Material loan,
    // before the first receiver drain, child close or Block reset.
    void preflight_close() const;
    void require_owned(bool, const char*) const;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

class NativePostEffectParentFrame final {
public:
    ~NativePostEffectParentFrame(); // Domain-owned metadata; never self-disposes
    NativePostEffectParentFrame(const NativePostEffectParentFrame&) = delete;
    NativePostEffectParentFrame& operator=(const NativePostEffectParentFrame&) = delete;
    NativePostEffectParentFrame(NativePostEffectParentFrame&&) = delete;
    NativePostEffectParentFrame& operator=(NativePostEffectParentFrame&&) = delete;
    bool requires_process_retention() const noexcept;
    void close_after_host_quiescence();
private:
    friend class NativePostEffectParentDomain;
    friend class NativePostEffectOwner;
    friend class NativePostEffectReference;
    friend class NativePostEffect20Owner;
    friend class NativePostEffect20Reference;
    friend class NativePostEffectConstructionBlock;
    friend class NativePostEffect20ConstructionBlock;
    friend class NativeRenderObservedZeroAccess;
    // Definitions remain private to the two actual payload provider TUs.
    // They may continue only the real returned-zero, already-resolved edge.
    friend class NativePostEffect20MaterialObservedZeroIssuer;
    friend class NativePostEffect24MaterialObservedZeroIssuer;
    friend void* construct_native_post_effect_00b4e840(void*, std::size_t,
        NativeString&, std::uint32_t, const void*, NativePostEffectConstructionBlock&);
    friend void* construct_native_post_effect20_00b4e470(void*, std::size_t,
        NativeString&, std::uint32_t, const void*, NativePostEffect20ConstructionBlock&);
    friend void* construct_native_post_effect20_00b4e470(void*, std::size_t,
        const NativePostEffect20ArgumentView&, NativePostEffect20ConstructionBlock&);
    friend void* create_native_post_effect_parent20_00b4e470(NativePostEffectParentFrame&,
        NativeString&, std::uint32_t, const void*);
    friend void* create_native_post_effect_parent20_00b4e470(NativePostEffectParentFrame&,
        const NativePostEffect20ArgumentView&);
    friend void* create_native_post_effect_parent24_00b4e840(NativePostEffectParentFrame&,
        NativeString&, std::uint32_t, const void*);
    friend void release_native_post_effect_parent_creator(NativePostEffectParentFrame&,
        long (__stdcall*)(volatile long*));
    friend void* delete_native_post_effect_parent20_scalar_00b4e430(
        NativePostEffectParentFrame&, const void*);
    friend void* delete_native_post_effect_parent24_scalar_00b4e450(
        NativePostEffectParentFrame&, const void*);

    NativePostEffectParentFrame(NativePostEffectParentDomain&,
        NativePostEffect20ConstructionBlock&);
    NativePostEffectParentFrame(NativePostEffectParentDomain&,
        NativePostEffectConstructionBlock&);

    // Only the exact existing constructor's private hook sites issue these.
    // Both are no-allocation/no-lock/no-Native-read observations. The atomic
    // hook is AFTER its actual placement1; normal-return precedes host tails.
    void native_atomic_born(void* same_actual) noexcept;
    void native_constructor_returned(void* same_actual) noexcept;
    bool native_constructor_has_returned() const noexcept;
    void publish_after_native_constructor();
    RenderCommandReference& canonical_after_publication();
    void ordinary_virtual_zero(RenderCommandReference&);

    // Exact Root Common interface. No public action/receipt setter exists.
    void begin_cpp_count_use(RenderCommandReference&);
    void end_cpp_count_use() noexcept;
    void hold_observed_zero(NativeRenderObservedZeroFrame&, RenderCommandReference&);
    void invoke_observed_zero(void* captured_actual, RenderCommandReference&);
    void release_observed_zero_hold(bool source_failed) noexcept;
    void record_source_failure() noexcept;

    void require_same_canonical(RenderCommandReference&) const;
    void require_healthy() const;
    void require_owned(bool, const char*) const;
    static void record_domain_failure(NativePostEffectParentDomain&) noexcept;
    void prepare_after_link();
    void* construct20(NativeString&, std::uint32_t, const void*);
    void* construct20(const NativePostEffect20ArgumentView&);
    void* construct24(NativeString&, std::uint32_t, const void*);
    NativePostEffect20Owner& owner20_after_publication();
    NativePostEffectOwner& owner24_after_publication();
    void begin_outer_call();
    void end_outer_call() noexcept;
    void cancel_normal_null_allocation();
    void* allocate_parent();
    void* scalar(const void* actual_flags_home);
    void source_constructor_returned();
    void release_creator(long (__stdcall* captured_decrement)(volatile long*));
    void end_cpp_parent_payload() noexcept;
    void notify_block_retired() noexcept;

    // Root5's Material Domain is validated against BOTH construction contexts
    // by the Domain constructor, before Block.prepare or Native effects.
    // Association follows Block.prepare and this Frame's retained attachment.
    // Root-approved H3 amendment: authentic selected entry BEFORE Native effects.
    void require_material_constructor_operation(void* actual) const;
    void prepare_material_caller(NativeMaterialFrame&,
        NativeMaterialFactoryAcquired&, NativeMaterialEffectCacheContext&);
    // Exact post-store B4E5E1/B4E9A5 notice, no field/count reread or retain.
    void material_field_published(NativeMaterialStorage* stored_actual) noexcept;
    void set_material_parameter_owner(NativeMaterialStorage&, void* incoming,
        std::uint8_t retain_byte, NativeRenderActualOwners&);
    // Existing decrement/profile/resolve/alias/type checks have already run.
    // Use Common invoke(reference, actual, terminal_zero_frame_), not dispatch.
    void invoke_material_observed_zero(RenderCommandReference&, void* captured_actual);
    void require_material_payload_operation() const;
    void preflight_material_caller_close() const;
    void close_material_caller_after_host_quiescence();
    bool material_caller_closed() const noexcept;
    void preflight_close() const;
    // Future CPP definitions: only the admitted constructor/scalar mutation
    // owner may propagate failure through its own receiver at the real fence.
    // Rejected calls and creator-nonzero paths never traverse these heads.
    class MaterialConstructorScope;
    class MaterialPayloadScope;

    struct Impl;
    std::unique_ptr<Impl> impl_;
    NativeRenderActualOwnerRegistry::ExactRecord exact_;
    NativeRenderObservedZeroHold observed_zero_hold_;
    // Set once after retained attachment; immutable association thereafter.
    // Domain owns this Material Frame; this pointer does not extend any loan.
    NativeMaterialFrame* material_frame_{};
    // Exactly one B18A40 call in this actual Parent construction attempt.
    // Lives cold through the whole constructor/outer catch and verified close.
    NativeRenderActualOwnerReleaseObservation material_parameter_release_;
    // Persistent payload-material receiver, distinct from the setter receiver.
    NativeRenderObservedZeroFrame terminal_zero_frame_;
};

// Ordinary Source producer APIs, using the genuine BF681B-role supplier with
// exact20h/24h extent. They do not initialize +4 before the existing Native
// constructor does. Every existing constructor mode/input path is forwarded.
void* create_native_post_effect_parent20_00b4e470(NativePostEffectParentFrame&,
    NativeString&, std::uint32_t vertex_count, const void* optional_size_input);
void* create_native_post_effect_parent20_00b4e470(NativePostEffectParentFrame&,
    const NativePostEffect20ArgumentView&);
void* create_native_post_effect_parent24_00b4e840(NativePostEffectParentFrame&,
    NativeString&, std::uint32_t primitive_count, const void* optional_size_input);
void release_native_post_effect_parent_creator(NativePostEffectParentFrame&,
    long (__stdcall* captured_decrement)(volatile long*));

// A direct-scalar callable is a valid-input operation, NOT an ownership issuer.
// Its actual caller must independently own that Native action and establish
// borrower quiescence; constructor return/public progress does not grant it.
// The HOME is a real addressable4B caller object/representation view. Only its
// late BYTE is loaded, after full payload normal return; no early value copy.
// flags0 leaves physical allocation debt and therefore cannot close the Frame.
void* delete_native_post_effect_parent20_scalar_00b4e430(
    NativePostEffectParentFrame&, const void* actual_flags_home);
void* delete_native_post_effect_parent24_scalar_00b4e450(
    NativePostEffectParentFrame&, const void* actual_flags_home);
} // namespace bsp
