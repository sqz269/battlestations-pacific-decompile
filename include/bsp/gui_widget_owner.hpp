#pragma once
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_widget_scene.hpp"
#include "bsp/native_model_owner.hpp"
#include <functional>
#include <unordered_map>

namespace bsp {
class GuiWidgetOwner;
class GuiWidgetOwnerRuntime;

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
    // Existing base types use00AA8530; cGuiLayer overrides the current slot34.
    virtual void set_visible34(GuiWidgetOwner&, bool);
    // Base and supported Icon/FrameBox readers have no pre-base continuation.
    // cGuiLayer00AC4C50 acquires its camera/scene BEFORE00AAA710.
    virtual void before_properties(GuiWidgetOwner&, const GuiTable&) {}
    // Derived destruction precedes the base00AA9730 node/tree release.
    virtual void before_scene_release(GuiWidgetOwner&) {}
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
};

// Reconstructed logical widget owner, not a raw100h ABI replacement. Existing
// GuiLayoutWidget holds the one tree and material fields. No private tree/map
// of replacement nodes is constructed. Additional fields cover native offsets
// absent from that projection; constructor-unwritten fields are marked below.
struct GuiWidgetBaseExtraFields {
    std::int32_t references_04{1};
    float fields_30_44[6]{};
    bool byte_79{};
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
    GuiLayoutWidget& layout() noexcept { return layout_; }
    GuiWidgetSceneFlags& scene_flags() noexcept { return scene_; }
    GuiWidgetBaseExtraFields& extra_fields() noexcept { return extra_; }
    NativeNodeBinding* node_binding() noexcept { return node_; }
    NativeModelReference* model_reference() noexcept;
    GuiWidgetTypeImplementation& implementation();

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
    void set_position_00aa7dc0(const GuiWidgetPoint&);
    void release_scene_nodes_00aa8320();
private:
    friend class GuiWidgetOwnerRuntime;
    GuiWidgetOwner(GuiLayoutWidget&, GuiWidgetOwnerRuntime&);
    GuiLayoutWidget& layout_;
    GuiWidgetOwnerRuntime& runtime_;
    GuiWidgetSceneFlags scene_;
    GuiWidgetBaseExtraFields extra_;
    NativeNodeBinding* node_{};
    std::unique_ptr<GuiWidgetTypeImplementation> implementation_;
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
    // Root native node belongs to the parent page owner, and can be188h group.
    GuiWidgetOwner& construct_root(GuiLayoutWidget&, NativeNodeBinding&);
    //00AA6640 standalone sequence; loader calls construct_child then74 only
    // AFTER attaching widget and node, matching its distinct inline sequence.
    GuiWidgetOwner& create_with_scene_00aa6640(GuiLayoutWidget&);
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
    struct ModelRecord;
    GuiWidgetOwnerEnvironment environment_;
    std::unordered_map<GuiLayoutWidget*, std::unique_ptr<GuiWidgetOwner>> widgets_;
    std::unordered_map<void*, std::unique_ptr<ModelRecord>> models_;
    GuiWidgetOwner& construct_base(GuiLayoutWidget&);
    NativeNodeBinding* create_model(const std::string&);
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
