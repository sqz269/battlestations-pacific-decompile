#pragma once
#include "bsp/native_mesh_section.hpp"
#include <memory>

namespace bsp {
class NativeMeshRetentionState;
class NativeMeshSectionLifetimeFrame;
class NativeMeshSectionLifetimeCall;
class NativeMeshSectionLifetimeIssuer;
class NativeRenderObservedZeroAccess;
class GuiNativeGeometryOwners;
class NativePostEffectParentDomain;
class NativeMeshSectionLayoutStorageAccess;
class NativeMeshSectionLayoutStorageLifetimeCalls;
struct NativeMeshSectionLayoutStorageContext;

// Actual outer Source last-use scope. Forward this SAME nonmoving scope through
// leaf providers; keep it through postcall/health/catch and the real caller tail.
// Destruction ends metadata activity only, never drains or performs Native work.
class NativeMeshSectionLifetimeSourceScope final {
public:
    NativeMeshSectionLifetimeSourceScope() noexcept;
    ~NativeMeshSectionLifetimeSourceScope() noexcept;
    NativeMeshSectionLifetimeSourceScope(const NativeMeshSectionLifetimeSourceScope&) = delete;
    NativeMeshSectionLifetimeSourceScope& operator=(const NativeMeshSectionLifetimeSourceScope&) = delete;
    void fail() noexcept; // metadata only; actual scope end propagates owned heads
private:
    friend class NativeMeshSectionLifetimeFrame;
    NativeMeshSectionLifetimeCall* first_{};
    const int uncaught_;
    bool failed_{}, ended_{};
};

// Stable one-shot caller receiver, reserved BEFORE this invocation's effects.
// Public diagnostics do not issue Native birth, zero, adoption or death facts.
class NativeMeshSectionLifetimeCall final {
public:
    ~NativeMeshSectionLifetimeCall();
    NativeMeshSectionLifetimeCall(const NativeMeshSectionLifetimeCall&) = delete;
    NativeMeshSectionLifetimeCall& operator=(const NativeMeshSectionLifetimeCall&) = delete;
    bool requires_process_retention() const noexcept;
private:
    friend class NativeMeshSectionLifetimeFrame;
    friend class NativeMeshSectionLifetimeSourceScope;
    explicit NativeMeshSectionLifetimeCall(NativeMeshSectionLifetimeFrame&, bool terminal);
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

class NativeMeshSectionLifetimeDomain final {
public:
    NativeMeshSectionLifetimeDomain(NativeMeshSectionEnvironment&,
        NativeRenderActualOwnerRegistry&, const volatile std::uint32_t& actual_ce4970,
        NativeMeshRetentionState& same_graph);
    ~NativeMeshSectionLifetimeDomain(); // metadata only; real owner retains debt
    NativeMeshSectionLifetimeDomain(const NativeMeshSectionLifetimeDomain&) = delete;
    NativeMeshSectionLifetimeDomain& operator=(const NativeMeshSectionLifetimeDomain&) = delete;
    NativeMeshSectionLifetimeFrame& prepare();
    bool requires_process_retention() const noexcept;
    bool has_retained_records() const noexcept;
    // Independent actual owner/native/worker quiescence, not a counter proof.
    void close_after_host_quiescence();
private:
    friend class NativeMeshSectionLifetimeFrame;
    friend class NativePostEffectParentDomain;
    friend class GuiNativeGeometryOwners;
    // Compatibility only: actual owning friends supply the real borrowed
    // roles. Matching addresses issue no birth, credit, borrow or owner fence.
    void require_owner_binding(NativeMeshSectionEnvironment&,
        NativeRenderActualOwnerRegistry&, const volatile std::uint32_t& actual_ce4970,
        NativeMeshRetentionState&) const;
    // Final no-new-entry owner fence ONLY, never a healthy-live readiness
    // probe. Seals preparation and checks every record without any drain.
    void preflight_close_after_host_quiescence();
    void require_owned(bool, const char*) const;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

class NativeMeshSectionLifetimeFrame final {
public:
    using Call = NativeMeshSectionLifetimeCall;
    using SourceScope = NativeMeshSectionLifetimeSourceScope;
    ~NativeMeshSectionLifetimeFrame();
    NativeMeshSectionLifetimeFrame(const NativeMeshSectionLifetimeFrame&) = delete;
    NativeMeshSectionLifetimeFrame& operator=(const NativeMeshSectionLifetimeFrame&) = delete;
    // Borrowed same canonical: caller separately owns its actual Native credit.
    NativeMeshSectionReference& canonical();
    Call& prepare_call(SourceScope& actual_outer_source);
    bool requires_process_retention() const noexcept;
    void close_after_host_quiescence();
private:
    friend class NativeMeshSectionLifetimeDomain;
    friend class NativeMeshSectionLifetimeCall;
    friend class NativeMeshSectionLifetimeSourceScope;
    friend class NativeMeshSectionLifetimeIssuer;
    friend class NativeMeshSectionLifetimeAccess;
    friend class NativeMeshSectionReference;
    friend class NativeRenderObservedZeroAccess;
    friend class GuiNativeGeometryOwners; // authentic existing clone raw-return catch only
    friend class NativeMeshSectionLayoutStorageAccess;
    friend NativeMeshSectionStorage* create_native_mesh_section_00533fa0(NativeMeshSectionLifetimeFrame&,Call&);
    friend NativeMeshSectionStorage* create_native_mesh_section_clone_00b85ef0(NativeMeshSectionLifetimeFrame&,Call&,const NativeMeshSectionStorage&);
    friend void release_native_mesh_section_creator(NativeMeshSectionLifetimeFrame&,Call&,long (__stdcall*)(volatile long*));
    friend void release_native_mesh_section_reference(NativeMeshSectionLifetimeFrame&,Call&);
    friend NativeMeshSectionStorage* delete_native_mesh_section_00b86690(NativeMeshSectionLifetimeFrame&,Call&,const void*);
    friend void destroy_native_mesh_section_00b86420(NativeMeshSectionLifetimeFrame&,Call&);
    friend void set_native_mesh_section_material_00b864c0(NativeMeshSectionLifetimeFrame&,Call&,void*);
    friend void set_native_mesh_section_vertex_stream_00b86500(NativeMeshSectionLifetimeFrame&,Call&,std::int32_t,void*);
    friend void set_native_mesh_section_vertex_layout_00b86650(NativeMeshSectionLifetimeFrame&,Call&,void*);
    friend void set_native_mesh_section_instance_generator_binding_00b417e0(NativeMeshSectionLifetimeFrame&,Call&,void*);
    friend void append_native_mesh_section_vertex_stream_00b85b80(NativeMeshSectionLifetimeFrame&,Call&,void*);
    friend void set_native_mesh_section_indexed_00b85600(NativeMeshSectionLifetimeFrame&,Call&,std::uint8_t);
    friend void clear_native_mesh_section_vertex_streams_00b86550(NativeMeshSectionLifetimeFrame&,Call&);
    friend void clear_native_mesh_section_resources_00b86390(NativeMeshSectionLifetimeFrame&,Call&);
    friend void rebuild_native_mesh_section_vertex_layout_00b865a0(NativeMeshSectionLifetimeFrame&,Call&,void*,NativeMeshSectionLayoutServices&);
    friend void clear_native_mesh_section_vertex_streams_storage_00b86550(NativeMeshSectionLifetimeFrame&,Call&,
        const NativeMeshSectionLayoutStorageContext&,NativeMeshSectionLayoutStorageLifetimeCalls&);
    friend void rebuild_native_mesh_section_vertex_layout_storage_00b865a0(NativeMeshSectionLifetimeFrame&,Call&,
        const volatile std::uint32_t&,void*,const NativeMeshSectionLayoutStorageContext&,void**,
        NativeMeshSectionLayoutStorageLifetimeCalls&);

    enum class Operation : std::uint32_t { create=1, copy, material, stream, layout,
        generator, append, indexed, clear_streams, clear_resources, rebuild,
        direct_payload, scalar, creator, reference, clear_storage, rebuild_storage };
    explicit NativeMeshSectionLifetimeFrame(NativeMeshSectionLifetimeDomain&);
    void prepare_after_link();
    void require_owned(bool,const char*) const;
    void require_healthy() const;
    static void record_domain_failure(NativeMeshSectionLifetimeDomain&) noexcept;
    void record_source_failure() noexcept; // metadata ONLY, never foreign heads
    void begin_cpp_count_use(RenderCommandReference&);
    void end_cpp_count_use() noexcept;
    void hold_observed_zero(NativeRenderObservedZeroFrame&,RenderCommandReference&);
    void invoke_observed_zero(void*,RenderCommandReference&);
    void release_observed_zero_hold(bool) noexcept;
    void ordinary_virtual_zero(RenderCommandReference&);
    void require_same_canonical(RenderCommandReference&) const;
    // Real ongoing Native payload borrow, including callback field mutation.
    // Grants no C++ count/zero/scalar action and ends at actual payload return.
    void require_live_payload_borrow() const;
    void begin_outer_call();
    void end_outer_call() noexcept;
    void begin_operation(Call&,std::uint32_t);
    void begin_call_activity(Call&) noexcept;
    void end_call_activity(Call&) noexcept;
    void fail_call_owned(Call&) noexcept;
    void release_child(Call&,NativeRenderActualOwners&,void*,std::size_t);
    void release_storage_child(Call&,void*,const NativeMeshSectionLayoutStorageContext&,
        NativeMeshSectionLayoutStorageLifetimeCalls&);
    void preflight_close() const;
    class OperationScope;
    class PayloadScope;

    void native_allocation_entered() noexcept;
    void native_allocation_returned(void*);
    void native_constructor_entered(void*);
    void cpp_prefix_born(NativeMeshSectionStorage&) noexcept;
    void native_count1_born(NativeMeshSectionStorage&) noexcept;
    void native_constructor_returned(NativeMeshSectionStorage&) noexcept;
    void source_constructor_escaped() noexcept;
    bool may_select_constructor_raw_return() const noexcept;
    void selected_constructor_raw_return(void*,NativeMeshSectionPool&);
    void publish_after_native_constructor();
    NativeMeshSectionStorage* produce(Call&,const NativeMeshSectionStorage*);
    void mutate(Call&,std::uint32_t,void*,std::int32_t,NativeMeshSectionLayoutServices*);
    void mutate_storage(Call&,std::uint32_t,const volatile std::uint32_t*,void*,
        const NativeMeshSectionLayoutStorageContext&,void**,
        NativeMeshSectionLayoutStorageLifetimeCalls&);
    void release_creator(Call&,long (__stdcall*)(volatile long*));
    void release_reference(Call&);
    NativeMeshSectionStorage* scalar(Call&,const void*);
    void payload(Call&);
    struct Impl;
    std::unique_ptr<Impl> impl_;
    NativeRenderActualOwnerRegistry::ExactRecord exact_;
    NativeRenderObservedZeroHold observed_zero_hold_;
    NativeRenderObservedZeroFrame terminal_zero_frame_;
};

NativeMeshSectionStorage* create_native_mesh_section_00533fa0(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&);
NativeMeshSectionStorage* create_native_mesh_section_clone_00b85ef0(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&,const NativeMeshSectionStorage&);
void release_native_mesh_section_creator(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&,long (__stdcall* actual_captured_decrement)(volatile long*));
// Drop the real caller's independently retained Section credit, not a creator
// receipt. SAME actual observed-release helper decrements/resolves only once.
void release_native_mesh_section_reference(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&);
NativeMeshSectionStorage* delete_native_mesh_section_00b86690(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&,const void* actual_live_four_byte_flags_home);
void destroy_native_mesh_section_00b86420(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&);
void set_native_mesh_section_material_00b864c0(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&,void*);
void set_native_mesh_section_vertex_stream_00b86500(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&,std::int32_t,void*);
void set_native_mesh_section_vertex_layout_00b86650(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&,void*);
void set_native_mesh_section_instance_generator_binding_00b417e0(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&,void*);
void append_native_mesh_section_vertex_stream_00b85b80(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&,void*);
void set_native_mesh_section_indexed_00b85600(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&,std::uint8_t);
void clear_native_mesh_section_vertex_streams_00b86550(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&);
void clear_native_mesh_section_resources_00b86390(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&);
void rebuild_native_mesh_section_vertex_layout_00b865a0(NativeMeshSectionLifetimeFrame&,NativeMeshSectionLifetimeFrame::Call&,void*,NativeMeshSectionLayoutServices&);
} // namespace bsp
