#pragma once
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_widget_scene.hpp"
#include "bsp/gui_widget_base_lifetime.hpp"
#include "bsp/native_model_owner.hpp"
#include <functional>
#include <unordered_map>

namespace bsp {
class GuiWidgetOwner;
class GuiWidgetOwnerRuntime;
class GuiTextLifetime;
class GuiTextChildDeletion;
class GuiTimedEntryOwner;
class GuiWidgetClipRefreshOperation;
struct GuiWidgetClipRefreshServices;

// Derived companions operate on the owner's SAME layout/transform. Factory
// creation performs the derived constructor, before node binding/parenting.
// Every per-type call is required; unsupported types must throw explicitly.
class GuiWidgetTypeImplementation {
public:
    virtual ~GuiWidgetTypeImplementation() = default;
    virtual void constructed74(GuiWidgetOwner&) = 0;
    virtual void properties_bound(GuiWidgetOwner&, const GuiTable&) = 0;
    virtual void loaded78(GuiWidgetOwner&) = 0;
    virtual void set_active60(GuiWidgetOwner&, bool) = 0;
    virtual bool is_visible38(GuiWidgetOwner&) = 0;
    virtual void visibility_changed3c(GuiWidgetOwner&, bool) = 0;
    // Current70 is required by recursive clip refresh. Profiles without an
    // established implementation fail explicitly, never silently complete it.
    virtual void refresh_clip70(GuiWidgetOwner&);
    virtual float* read_color54(GuiWidgetOwner&, float (&)[4]);
    virtual void set_alpha4c(GuiWidgetOwner&, float);
    virtual std::int32_t type5c(GuiWidgetOwner&);
    // Host-only preflight for retained C++ continuations before scalar deletion
    // starts. Not another native slot or side effect. Types with pending frames
    // reject deletion here; existing types have no such continuation metadata.
    virtual void before_scalar_deletion4(GuiWidgetOwner&) {}
    // Existing base types use00AA8530; cGuiLayer overrides the current slot34.
    virtual void set_visible34(GuiWidgetOwner&, bool);
    // Base and supported Icon/FrameBox readers have no pre-base continuation.
    // cGuiLayer00AC4C50 acquires its camera/scene BEFORE00AAA710.
    virtual void before_properties(GuiWidgetOwner&, const GuiTable&) {}
    // Derived destruction precedes the base00AA9730 node/tree release.
    virtual void before_scene_release(GuiWidgetOwner&) {}
    //00AA8372/00AA837B derived type-query branch, after child+20 and before
    // primary unlink. Text supplies the live descriptor predicate and AB73B0.
    // Existing supported profiles own no secondary scene nodes.
    virtual void release_secondary_scene_nodes(GuiWidgetOwner&) {}
};
using GuiWidgetImplementationFactory = std::function<
    std::unique_ptr<GuiWidgetTypeImplementation>(GuiWidgetOwner&)>;

// Required actual engine operations. No second node hierarchy or renderer.
class GuiWidgetNativeCalls {
public:
    virtual ~GuiWidgetNativeCalls() = default;
    // Resolve non-widget descendants as well, using actual existing bindings.
    virtual NativeNodeBinding& resolve_node(CameraTransform&) = 0;
    virtual void set_parent_00b6e680(NativeNodeBinding& child,
        NativeNodeBinding* parent) = 0;
};
struct GuiWidgetOwnerEnvironment {
    NativeModelEnvironment& models;
    GuiWidgetNativeCalls& native;
    GuiWidgetImplementationFactory make_type;
    // Configure after building the SAME runtime/material service domain.
    GuiWidgetClipRefreshServices* clip{};
};

// Reconstructed logical widget owner, not a raw100h ABI replacement. Existing
// GuiLayoutWidget holds the one tree and material fields. No private tree/map
// of replacement nodes is constructed. Additional fields cover native offsets
// absent from that projection; constructor-unwritten fields are marked below.
struct GuiWidgetBaseExtraFields {
    std::int32_t references_04{1};
    float fields_30_44[6]{};
    std::uint8_t byte_79{};
    float fields_7c_80[2]{};
    void* pointers_88_90[3]{};
    float overbright_94{};
    bool byte_d4{};
    void* pointer_d8{};
    void* layout_listener_dc{};
    // Native00AA9390 leaves+E8 unwritten;00AA9F10 assigns it before borrowing.
    // Do not read until clip registration has established this SAME field.
    float clip_enabled_e8;
};
class GuiWidgetOwner final {
public:
    ~GuiWidgetOwner() noexcept;
    GuiLayoutWidget& layout() noexcept { return layout_; }
    GuiWidgetSceneFlags& scene_flags() noexcept { return scene_; }
    GuiWidgetBaseExtraFields& extra_fields() noexcept { return extra_; }
    NativeNodeBinding* node_binding() noexcept { return node_; }
    NativeModelReference* model_reference() noexcept;
    // Borrowed canonical companion, published before Text constructor callbacks.
    // Not another Text state or factory registration. Null after typed teardown.
    GuiTextLifetime* text_lifetime() noexcept { return text_lifetime_; }
    GuiWidgetTypeImplementation& implementation();
    GuiWidgetOwnerRuntime& runtime() noexcept { return runtime_; }
    const GuiWidgetBaseLifetimeState& base_lifetime_state() const noexcept { return base_lifetime_; }
    GuiTimedEntryOwner& timed_entries(const volatile float& one_00d7a24c);
    void require_timed_entry_ownership() const;
    void retire_timed_entries_00aa9730_fragment();
    void base_refresh_clip70_00aaa3e0();
    void resume_base_clip_after_child70();
    bool has_pending_base_clip() const noexcept;
    void require_no_active_owned_operation() const;

    void bind_scene_00aa6720(NativeNodeBinding*) noexcept;
    void base_constructed74_00a9ac00() noexcept; // proven single RET, no Ghidra function
    void base_loaded78_00aa7170();
    void base_set_active60_00aa6a30(bool) noexcept;
    bool base_is_visible38_00a9e0d0() const noexcept;
    void base_visibility_changed3c_00a9e100(bool) noexcept; // proven RET4
    void set_visible_00aa8530(bool);
    void set_visible34(bool); // actual current virtual34, including cGuiLayer
    void propagate_visibility_00aa8450(const GuiWidgetVisibilityArgs&);
    void recompose_00aa7220();
    void refresh_bounds_00aa70e0();
    void refresh_clip70();
    void set_position_00aa7dc0(const GuiWidgetPoint&);
    void release_scene_nodes_00aa8320();
private:
    friend class GuiWidgetOwnerRuntime;
    friend class GuiTextLifetime;
    friend class GuiTextChildDeletion;
    friend void destroy_gui_widget_base_00aa9730(GuiWidgetOwner&, GuiTextChildDeletion&);
    GuiWidgetOwner(GuiLayoutWidget&, GuiWidgetOwnerRuntime&);
    GuiLayoutWidget& layout_;
    GuiWidgetOwnerRuntime& runtime_;
    GuiWidgetSceneFlags scene_;
    GuiWidgetBaseExtraFields extra_;
    NativeNodeBinding* node_{};
    GuiTextLifetime* text_lifetime_{};
    std::unique_ptr<GuiWidgetTypeImplementation> implementation_;
    std::unique_ptr<GuiTimedEntryOwner> timed_entries_;
    std::unique_ptr<GuiWidgetClipRefreshOperation> base_clip_;
    GuiWidgetClipRefreshServices* base_clip_services_{};
    GuiWidgetBaseLifetimeState base_lifetime_;
    bool scene_release_active_{};
};

// Lifetime associations only. GUI storage is owned by GuiLayoutPage; native
// models are real188h slots from the supplied canonical pool. The host must
// release GUI nodes/companions before destroying layouts and drain any retained
// render references before destroying this runtime/environment.
class GuiWidgetOwnerRuntime final {
public:
    explicit GuiWidgetOwnerRuntime(GuiWidgetOwnerEnvironment);
    ~GuiWidgetOwnerRuntime();
    GuiWidgetOwnerRuntime(const GuiWidgetOwnerRuntime&) = delete;
    GuiWidgetOwnerRuntime& operator=(const GuiWidgetOwnerRuntime&) = delete;
    GuiWidgetOwner& construct_child_00aa6560(GuiLayoutWidget&);
    // AB9650 construction used by AB98F0 before copying the template model.
    // Creates the SAME base/type companion, leaving the primary model null;
    // does not allocate an ordinary named model, attach, or invoke74/78.
    GuiWidgetOwner& construct_unbound_text_00ab9650(GuiLayoutWidget&);
    // Root native node belongs to the parent page owner, and can be188h group.
    GuiWidgetOwner& construct_root(GuiLayoutWidget&, NativeNodeBinding&);
    //00AA6640 standalone sequence; loader calls construct_child then74 only
    // AFTER attaching widget and node, matching its distinct inline sequence.
    GuiWidgetOwner& create_with_scene_00aa6640(GuiLayoutWidget&);
    //00AB8530 auxiliary drawable fragment: reuse this runtime's SAME model
    // pool/owner/reference map; publish to the caller's live +188 association
    // before releasing the temporary name. Returns one creator reference,
    // retired through the existing node lifetime binding, not a second owner.
    void create_auxiliary_model_00ab8530_fragment(NativeNodeBinding*& publication,
        const std::string& name);
    // B752D1..B7530A: allocate from the SAME Model pool, then construct from
    // the source's CURRENT actual name header. No temporary name or source
    // snapshot. Register the created owner/reference in this runtime's one
    // model map and return ONE creator reference. Caller keeps it across all
    // later clone effects; constructor failure returns its raw slot. Source
    // must be a live Model already registered here and survive callbacks.
    // Null pool allocation throws: the native subsequent null dereference is
    // outside this C++ interface. Host registration failure is not native EH.
    NativeModelReference& create_model_clone_destination_00b752b0_fragment(
        NativeModelOwner& source);
    GuiWidgetOwner& owner(GuiLayoutWidget&) const;
    GuiWidgetOwner& owner(GuiWidgetTransform&) const;
    NativeNodeBinding& node(std::uint32_t actual_identity) const;
    void set_node_parent(std::uint32_t child, std::uint32_t parent);
    void constructed74(GuiLayoutWidget&);
    void before_properties(GuiLayoutWidget&, const GuiTable&);
    void base_properties_bound(GuiLayoutWidget&);
    void properties_bound(GuiLayoutWidget&, const GuiTable&);
    void loaded78(GuiLayoutWidget&);
    // AA31F0 disposal fragment: recursive current20 scene release BEFORE
    // derived destruction and the base AA9730 release pass, then erase
    // companions. Leaves layout ownership
    // and its existing child lists with the page. Never dereference after page dies.
    void retire_tree(GuiLayoutWidget&);
    std::size_t retained_model_count() const noexcept { return models_.size(); }
    GuiWidgetOwnerEnvironment& environment() noexcept { return environment_; }
private:
    friend class GuiWidgetOwner;
    friend class GuiTextChildDeletion;
    friend void destroy_gui_widget_base_00aa9730(GuiWidgetOwner&, GuiTextChildDeletion&);
    struct ModelRecord;
    GuiWidgetOwnerEnvironment environment_;
    std::unordered_map<GuiLayoutWidget*, std::unique_ptr<GuiWidgetOwner>> widgets_;
    std::unordered_map<void*, std::unique_ptr<ModelRecord>> models_;
    GuiWidgetOwner& construct_base(GuiLayoutWidget&);
    NativeNodeBinding* create_model(const std::string&,
        NativeNodeBinding** publication_before_name_release = nullptr);
    void stamp_visibility(NativeNodeBinding&, float, bool);
    void propagate_visibility(GuiWidgetOwner&, const GuiWidgetVisibilityArgs&);
    void erase_tree(GuiLayoutWidget&);
    static void retire_model(void*, NativeModelReference&) noexcept;
};

// Full B64780 arithmetic, ECX destination/EDX float pointer, RET. x87 FSIN,
// FCOS and separate float32 spills; existing00413920 multiplies the matrices.
void build_gui_rotation_z_00b64780(CameraMatrix&, const float& angle);
// Actual storage accessors, not projected geometry adapters. Caller supplies
// the native model tail, geometry header with pointer-array+54 and element with
// writable floats+24..30. No allocation, bounds validation or ownership change.
void* gui_model_geometry_00b74640(const NativeModelTailStorage&, std::uint32_t unused) noexcept;
void* gui_geometry_element_00b732c0(const void* actual_geometry, std::int32_t index) noexcept;
void set_gui_element_bounds_00b855b0(void* actual_element, const GuiWidgetBounds&) noexcept;
} // namespace bsp
