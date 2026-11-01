#pragma once
#include "bsp/native_mesh_storage_lifetime.hpp"
#include <atomic>
#include <cstdint>
#include <memory>

namespace bsp {
namespace game { class GameNativeRendererApplication; }
class GuiNativeGeometryOwners;
class NativeMeshPreparedRecord;
class NativePostEffectConstructionBlock;
class NativePostEffect20ConstructionBlock;
class NativeMeshCanonicalRecord;
class NativeMeshCanonicalAccess;
class NativeMeshCanonicalProducerFrame;
class NativeMeshCanonicalBorrow;
class NativePostEffectModelFrame;

// Source metadata, owned by the actual renderer before its graphs and workers.
// This adds no Native field, counter, callback, exception policy or ownership.
class NativeMeshRetentionState final {
public:
    NativeMeshRetentionState() noexcept = default;
    NativeMeshRetentionState(const NativeMeshRetentionState&) = delete;
    NativeMeshRetentionState& operator=(const NativeMeshRetentionState&) = delete;
    bool requires_process_retention() const noexcept;
    bool has_retained_records() const noexcept;
private:
    friend class NativeMeshCanonicalRecord;
    friend class NativeMeshCanonicalAccess;
    friend class NativeMeshCanonicalDomain;
    friend class NativePostEffectModelFrame;
    friend class NativePostEffectParentFrame;
    friend class NativeMaterialFrame;
    friend class NativeMeshSectionLifetimeFrame;
    friend class game::GameNativeRendererApplication;
    friend class NativeRendererRawFailureBridge;
    std::atomic<bool> failed_{false};
    std::atomic<std::uint32_t> retained_{0};
    void fail() noexcept { failed_.store(true, std::memory_order_release); }
};

// One concrete SAME-pool/context/registry owner. No address/profile adoption.
// Its actual host must retain the complete graph on failed/unknown Native debt,
// and close only after workers, callbacks, native credits and C++ uses quiesce.
class NativeMeshCanonicalDomain final {
public:
    NativeMeshCanonicalDomain(NativeMeshEnvironment&, NativeMeshConstants,
        NativeRenderActualOwnerRegistry&, NativeMeshRetentionState&);
    ~NativeMeshCanonicalDomain() noexcept;
    NativeMeshCanonicalDomain(const NativeMeshCanonicalDomain&) = delete;
    NativeMeshCanonicalDomain& operator=(const NativeMeshCanonicalDomain&) = delete;
    NativeMeshCanonicalDomain(NativeMeshCanonicalDomain&&) = delete;
    NativeMeshCanonicalDomain& operator=(NativeMeshCanonicalDomain&&) = delete;
    bool requires_process_retention() const noexcept;
    bool has_retained_records() const noexcept;
    // Caller supplies actual graph quiescence; this is no Native drain or proof
    // from size/empty. Throws without collecting unresolved records.
    void close_at_quiescence();
private:
    friend class NativeMeshCanonicalRecord;
    friend class NativeMeshCanonicalAccess;
    friend class NativeMeshCanonicalProducerFrame;
    friend class NativePostEffectModelFrame;
    friend class NativePostEffectParentFrame;
    struct Impl;
    NativeMeshEnvironment& environment_;
    NativeRenderActualOwnerRegistry& registry_;
    NativeMeshRetentionState& retention_;
    const NativeMeshConstants constants_;
    NativeMeshStorageContext context_;
    std::unique_ptr<Impl> impl_;
    NativeMeshCanonicalRecord& reserve(const void* actual_producer);
};

// Concrete stable producer frame. Construction reserves C++ custody BEFORE any
// allocating Native operation. It does not accept/adopt a raw Native address.
// Ending this frame neither destroys a Native payload nor erases its record.
class NativeMeshCanonicalProducerFrame final {
public:
    NativeMeshCanonicalProducerFrame(NativeMeshCanonicalDomain&,
        const void* exact_stable_producer_identity);
    ~NativeMeshCanonicalProducerFrame() noexcept = default;
    NativeMeshCanonicalProducerFrame(const NativeMeshCanonicalProducerFrame&) = delete;
    NativeMeshCanonicalProducerFrame& operator=(const NativeMeshCanonicalProducerFrame&) = delete;
    NativeMeshCanonicalProducerFrame(NativeMeshCanonicalProducerFrame&&) = delete;
    NativeMeshCanonicalProducerFrame& operator=(NativeMeshCanonicalProducerFrame&&) = delete;
    bool requires_process_retention() const noexcept;
private:
    friend class NativeMeshCanonicalAccess;
    friend class NativeMeshCanonicalRecord;
    friend class GuiNativeGeometryOwners;
    friend class NativeMeshPreparedRecord;
    friend class NativeMeshExternalProducer;
    friend class NativePostEffectConstructionBlock;
    friend class NativePostEffect20ConstructionBlock;
    NativeMeshCanonicalDomain& domain_;
    const void* const producer_;
    NativeMeshCanonicalRecord* record_{};
};

// C++ record pin only: it adds NO Native credit/retain or scalar permission.
// A raw caller already owns the actual positive credit/field use keeping the
// Native payload live, and keeps this pin through EVERY count/payload use.
class NativeMeshCanonicalBorrow final {
public:
    explicit NativeMeshCanonicalBorrow(NativeMeshReference&);
    ~NativeMeshCanonicalBorrow() noexcept;
    NativeMeshCanonicalBorrow(const NativeMeshCanonicalBorrow&) = delete;
    NativeMeshCanonicalBorrow& operator=(const NativeMeshCanonicalBorrow&) = delete;
    NativeMeshCanonicalBorrow(NativeMeshCanonicalBorrow&&) noexcept;
    NativeMeshCanonicalBorrow& operator=(NativeMeshCanonicalBorrow&&) = delete;
    NativeMeshStorage& storage();
private:
    friend class NativeMeshCanonicalAccess;
    NativeMeshReference& reference_;
    NativeMeshCanonicalRecord* record_{};
    bool active_{true};
    int uncaught_on_entry_{};
};

// Private Source application bridge, not a public generic apply/permit API.
// Each friend is an actual owning producer or the existing release boundary.
class NativeMeshCanonicalAccess final {
private:
    friend class NativeMeshCanonicalDomain;
    friend class NativeMeshCanonicalRecord;
    friend class NativeMeshCanonicalProducerFrame;
    friend class NativeMeshCanonicalBorrow;
    friend class NativeMeshStorageAccess;
    friend class NativeMeshPayloadReleaseAccess;
    friend class NativeRenderObservedZeroAccess;
    friend class NativeMeshReference;
    friend class GuiNativeGeometryOwners;
    friend class NativeMeshPreparedRecord;
    friend class NativeMeshExternalProducer;
    friend class NativePostEffectConstructionBlock;
    friend class NativePostEffect20ConstructionBlock;
    friend void retain_render_command_reference(RenderCommandReference&) noexcept;
    friend void release_render_command_reference(RenderCommandReference&);
    friend void dispatch_native_render_actual_owner_zero(NativeRenderActualOwners&, void*);
    friend void release_native_render_actual_owner(NativeRenderActualOwners&, void*);
    friend void release_native_render_actual_owner_observed(NativeRenderActualOwners&,
        void*, NativeRenderActualOwnerReleaseObservation&);

    // SAME low Frame/receiver throughout; no callback or mode-dependent fallback.
    static void begin_payload_receiver(NativeMeshCanonicalRecord&);
    static NativeMeshPayloadReleaseReceiver& payload_receiver(
        NativeMeshCanonicalRecord&, NativeMeshStorageFrame&);
    static void release_payload_child(NativeMeshPayloadReleaseReceiver&,
        NativeRenderActualOwners&, void* captured_actual);
    static void end_payload_receiver(NativeMeshCanonicalRecord&, bool source_failed) noexcept;
    static void preflight_payload_receiver(NativeMeshCanonicalRecord&, bool complete);
    static void drain_payload_receiver(NativeMeshCanonicalRecord&);
    static void require_domain_identity(NativeMeshCanonicalDomain&, NativeMeshEnvironment&,
        NativeRenderActualOwners&);
    static void require_domain_identity(NativeMeshCanonicalDomain&, NativeMeshEnvironment&,
        NativeRenderActualOwners&, NativeMeshConstants);
    static void retain_source_graph(NativeMeshCanonicalDomain&) noexcept;
    static void bind_block(NativeMeshCanonicalProducerFrame&, NativePostEffectConstructionBlock&);
    static void bind_block(NativeMeshCanonicalProducerFrame&, NativePostEffect20ConstructionBlock&);
    static void* allocate_explicit(NativeMeshCanonicalProducerFrame&);
    static void* allocate_static(NativeMeshCanonicalProducerFrame&);
    static NativeMeshStorage* construct(NativeMeshCanonicalProducerFrame&);
    // Established genuine normal constructor + reserved exact node only.
    // No allocation, mutex, validation, count/profile read or Native callback.
    static NativeMeshReference& publish(NativeMeshCanonicalProducerFrame&) noexcept;
    static NativeMeshReference* canonical(NativeMeshCanonicalProducerFrame&) noexcept;
    static bool source_retired(const NativeMeshCanonicalProducerFrame&) noexcept;
    // Only an actual original creator-consumption site may call this. Performs
    // the real existing decrement once, not a count sample/permission mint.
    static void release_creator(NativeMeshCanonicalProducerFrame&);
    static void release_creator(NativeMeshCanonicalProducerFrame&,
        long (__stdcall* captured_decrement)(volatile long*));
    static void transfer_creator_to_caller(NativeMeshCanonicalProducerFrame&) noexcept;
    // True only when the selected actual B72F70 returned this original slot;
    // born/entered-unknown constructor is retained with NO raw return.
    static bool return_unconstructed_static_slot(NativeMeshCanonicalProducerFrame&, void* captured_slot);
    // Genuine never-entered/normal-NULL attempt only; no detach/cancel.
    static void preflight_cancel_without_native_payload(NativeMeshCanonicalProducerFrame&);
    static void cancel_without_native_payload(NativeMeshCanonicalProducerFrame&);
    // Read only completion/active/hold metadata; never drain as a probe.
    // The actual complete caller/worker quiescence remains external.
    static void preflight_producer_link_at_quiescence(NativeMeshCanonicalProducerFrame&);
    static void close_producer_link_at_quiescence(NativeMeshCanonicalProducerFrame&);
    static void record_source_failure(NativeMeshCanonicalProducerFrame&) noexcept;

    static void require_cpp_count_access(NativeMeshReference&);
    static void hold_observed_zero(NativeMeshReference&, class NativeRenderObservedZeroFrame&);
    static void fail_observed_outer(NativeMeshCanonicalRecord&) noexcept;
    static void release_observed_outer(NativeMeshCanonicalRecord&) noexcept;
    static void preflight_observed_outer_at_quiescence(NativeMeshCanonicalRecord&);
    static void drain_observed_outer_at_quiescence(NativeMeshCanonicalRecord&);
    static void arm_observed_zero(NativeMeshReference&) noexcept;
    static void release_zero(NativeMeshReference&);
    static NativeMeshStorage& scoped_storage(NativeMeshReference&,
        NativeMeshCanonicalBorrow&);
    static NativeMeshStorage& reject_unscoped_storage(NativeMeshReference&);
    // Direct calls from the low scalar's actual private normal+last-HOME/end
    // continuation. No public callback, heap, mutex, Native read/write or throw.
    static void native_constructor_returned(NativeMeshCanonicalRecord&, NativeMeshStorage*) noexcept;
    static void native_payload_ended(NativeMeshCanonicalRecord&) noexcept;
    static void poison(NativeMeshCanonicalRecord&) noexcept;
};
} // namespace bsp
