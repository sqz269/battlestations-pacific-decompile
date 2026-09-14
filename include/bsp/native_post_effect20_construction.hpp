#pragma once
#include "bsp/native_post_effect_owner_20h.hpp"
#include "bsp/native_post_effect_draw_record.hpp"
#include "bsp/native_camera_reference.hpp"
#include "bsp/native_logical_vertex_owner.hpp"
#include "bsp/native_vertex_declaration_cache.hpp"
#include "bsp/gui_text_native_layout.hpp"
#include <array>
#include <optional>

namespace bsp {
class ActualNativeStringPoolStorage;
struct NativeTracelineRenderAccess;

// One application's existing canonical domains. All references and publication
// cells remain stable through every surviving child and host view. The GUI owner
// domain is used only for its non-retaining declaration registration/reuse path;
// its create_mesh/create_section/create_material rollback policies are not used.
struct NativePostEffect20ConstructionContext {
    NativePostEffect20OwnerContext& destruction;
    NativeMeshEnvironment& meshes;
    NativeMeshConstants mesh_constants;
    NativeMeshSectionEnvironment& sections;
    const volatile std::uint32_t& section_bounds_w_00ce4970;
    NativeMaterialDestructionAccess& materials;
    GuiNativeGeometryOwners& declaration_companions;
    NativeVertexDeclarationCacheContext& declarations;
    NativeLogicalVertexOwnerContext& streams;
    GuiTextNativeLayoutServices& layouts;
    NativeModelEnvironment& models;
    NativeCameraEnvironment& cameras;
    const NativeNodeRawConstants& node_constants;
    NativeViewportRegistry& viewports;
    NativeTracelineRenderAccess& draw_entries;
    NativeStringRawPoolContext& raw_strings;
    ActualNativeStringPoolStorage& strings;
    void* const volatile& actual_renderer_00f8d394;
    const volatile std::uint32_t* renderer_profile_00d5f0a8;
    const volatile std::uint32_t* surface_profile_00d619a0;
    const volatile std::uint32_t* viewport_profile_00d5e5f8;
    const char* vertex_format_00d61ef8;
    const char* model_name_00d61ee4;
    const char* camera_prefix_00d61ed0;
};

// HOST diagnostics, not a native ownership map. Created identities remain
// identifiable after failures; no field grants permission to replay a release.
// Native return/decrement obligations are consumed before their calls. Surviving
// creators and host binding failures need explicit external disposition. Borrowed
// reference pointers are published here before registration can fail, permitting
// disposition through the exact unregistered companion; they add no ownership.
struct NativePostEffect20ConstructionAcquired {
    void* actual_owner{};
    bool native_completed{};
    int native_state{-1};
    void* frame_created{};
    void* stream_created{};
    NativeMeshStorage* mesh_created{};
    NativeMaterialStorage* material_created{};
    NativeMeshSectionStorage* section_created{};
    NativeMeshReference* mesh_reference{};
    NativeMeshSectionReference* section_reference{};
    NativeMaterialReference* material_reference{};
    NativeLogicalVertexReference* stream_reference{};
    NativeModelOwner* model_owner{};
    NativeModelReference* model_reference{};
    NativeCameraOwner* camera_owner{};
    NativeCameraReference* camera_reference{};
    NativePostEffect20Owner* completed_owner{};
    NativePostEffect20Reference* completed_reference{};
    NativeViewportOwner* viewport_creator{};
    void* draw_record_created{};
    GuiNativeDeclarationAcquired declaration;
    bool declaration_release_started{};
    bool stream_creator_release_started{};
    bool viewport_release_started{};
    bool section_creator_release_started{};
    bool mesh_creator_release_started{};
    void* last_string_return_data{};
    std::uint32_t last_string_return_bytes{};
    bool string_return_started{};
};

// Caller-owned, immovable storage prepared BEFORE B4E470. All optional owner/
// reference objects and both viewport views survive settled failed attempts.
// There is no automatic native cleanup or orphan recovery. Registration can
// allocate host metadata; only camera scene/lifetime and viewport credits are
// reserved up front. Failed native constructors alone return their raw slots.
class NativePostEffect20ConstructionBlock final {
public:
    enum class Phase { idle, preparing, prepared, executing, settled };
    NativePostEffect20ConstructionBlock() noexcept = default;
    ~NativePostEffect20ConstructionBlock() noexcept;
    NativePostEffect20ConstructionBlock(const NativePostEffect20ConstructionBlock&) = delete;
    NativePostEffect20ConstructionBlock& operator=(const NativePostEffect20ConstructionBlock&) = delete;
    NativePostEffect20ConstructionBlock(NativePostEffect20ConstructionBlock&&) = delete;
    NativePostEffect20ConstructionBlock& operator=(NativePostEffect20ConstructionBlock&&) = delete;
    void prepare(NativePostEffect20ConstructionContext&);
    void cancel_preparation() noexcept;
    Phase phase() const noexcept { return phase_; }
    const NativePostEffect20ConstructionAcquired& acquired() const noexcept { return acquired_; }
    // Does not release anything or recover a failed attempt. Caller proves all
    // raw creators and every semantic view/cache borrow have ended. References
    // must be retired, owners dead, and both viewport records nonlive. Unbound
    // completed-owner diagnostics cannot be silently discarded by this call.
    void reset_after_host_quiescence() noexcept;
private:
    friend void* construct_native_post_effect20_00b4e470(void*, std::size_t,
        NativeString&, std::uint32_t, const void*, NativePostEffect20ConstructionBlock&);
    enum Slot : std::size_t { mesh, section, material, stream, model, camera, owner, slot_count };
    void validate() const;
    void settle() noexcept;
    void bind(Slot, void*, RenderCommandReference&);
    void retired(Slot, RenderCommandReference&) noexcept;
    void string_cleanup(NativeString&);
    void return_captured_string(void*, std::uint32_t);
    void unwind(int&, void*&, void*&, void*&, std::uint32_t&);
    static void retire_mesh(void*, NativeMeshReference&) noexcept;
    static void retire_section(void*, NativeMeshSectionReference&) noexcept;
    static void retire_material(void*, NativeMaterialReference&) noexcept;
    static void retire_stream(void*, NativeLogicalVertexReference&) noexcept;
    static void retire_model(void*, NativeModelReference&) noexcept;
    static void retire_camera(void*, NativeCameraReference&) noexcept;
    static void retire_owner(void*, NativePostEffect20Reference&) noexcept;
    NativePostEffect20ConstructionContext* context_{};
    Phase phase_{Phase::idle};
    NativePostEffect20ConstructionAcquired acquired_;
    std::array<void*, slot_count> keys_{};
    std::array<RenderCommandReference*, slot_count> references_{};
    std::array<bool, slot_count> registered_{};
    std::array<bool, slot_count> retired_{};
    std::optional<NativeMeshReference> mesh_reference_;
    std::optional<NativeMeshSectionReference> section_reference_;
    std::optional<NativeMaterialReference> material_reference_;
    std::optional<NativeLogicalVertexReference> stream_reference_;
    std::optional<NativeModelOwner> model_owner_;
    std::optional<NativeModelReference> model_reference_;
    std::optional<NativeCameraOwner> camera_owner_;
    std::optional<NativeCameraReference> camera_reference_;
    std::optional<NativePostEffect20Owner> owner_;
    std::optional<NativePostEffect20Reference> owner_reference_;
    SceneAttachmentRuntime::BindingAdmission camera_scene_;
    GeneratedModelLifetimeRuntime::BindingAdmission camera_lifetime_;
    NativeViewportRegistry::Storage viewport_records_[2];
    NativeViewportRegistry::Admission viewport_admissions_[2];
    NativeString declaration_name_, model_name_, camera_name_;
};

// Complete B4E470 normal body and its logical twelve-state cleanup schedule.
// Original ECX actual20h/32-byte allocation; name/count/optional input stacked;
// EAX same allocation; RET0Ch. New C++ interface borrows prepared host storage.
// The optional nonnull domain is current D619A0 with exact B3CD20/B3CD10 getters;
// all 27 observed native calls pass null and vertex count 3. +04 atomic lifetime begins only at
// original B4E49E count1. No whole-object initialization or extra retain occurs.
// After native_completed, a binding diagnostic must NOT cause caller raw free
// or base cleanup. This is not native FH3, binary ABI, GPU or gameplay proof.
void* construct_native_post_effect20_00b4e470(void* actual_allocation,
    std::size_t allocation_bytes, NativeString& actual_effect_name,
    std::uint32_t vertex_count, const void* optional_size_input,
    NativePostEffect20ConstructionBlock&);

} // namespace bsp
