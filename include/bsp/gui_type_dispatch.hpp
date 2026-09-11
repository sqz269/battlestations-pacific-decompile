#pragma once
#include "bsp/gui_framebox.hpp"
#include "bsp/gui_layer.hpp"
#include "bsp/gui_widget_owner.hpp"

namespace bsp {
// Only the derived cGuiLayer fields. Base transform, tree, visibility propagation
// and native node binding remain in the SAME GuiWidgetOwner/GuiLayoutWidget.
// Semantic C++ ABI; no124h allocation or binary-compatible vtable is claimed.
struct GuiScreenLayerState {
    const void* scene_ec{};
    GuiCameraStore* store_f0{};
    bool owns_store_f4{}; // valid after the actual acquire78 callback
    bool visible_f5{};
    float field_f8{};
    std::int32_t applied_priority_fc{};
    std::string name_100;
    GuiCameraStoreKey key_108;
    std::uint8_t share_scene_120{}; // exact constructor byte, not normalized
    std::uint8_t reserved_121{};
};
struct GuiScreenLayerServices {
    // REQUIRED complete00AC59A0 over this derived state and this owner's base.
    // Must retain actual scene/camera/light/store resources and implement the
    // native shared-store early return, flags and priority-registration tail.
    std::function<void(GuiWidgetOwner&, GuiScreenLayerState&)> acquire_camera_store78_00ac59a0;
    std::function<void(NativeNodeBinding&, const void*)> bind_node_to_scene_00b6d890;
    std::function<bool()> visibility_hook_installed_00f8bf4c;
    std::function<void(std::string_view, bool)> visibility_hook_00f8bf4c;
    // REQUIRED derived00AC5480 before base scene-node release. Must not throw.
    // Handles partial acquisition, owned store removal/shared scene release;
    // the runtime keeps the one layout/node tree and releases that base itself.
    std::function<void(GuiWidgetOwner&, GuiScreenLayerState&)> release_derived_00ac5480;
};

// Concrete base slots verified in all four vtables. Group's reader00AC6FD0
// tail-jumps to00AAA710, so the loader already performed its entire body.
class GuiGroupTypeImplementation : public GuiWidgetTypeImplementation {
public:
    void constructed74(GuiWidgetOwner&) override;
    void properties_bound(GuiWidgetOwner&, const GuiTable&) override;
    void loaded78(GuiWidgetOwner&) override;
    void set_active60(GuiWidgetOwner&, bool) override;
    bool is_visible38(GuiWidgetOwner&) override;
    void visibility_changed3c(GuiWidgetOwner&, bool) override;
    void set_visible34(GuiWidgetOwner&, bool) override;
};
class GuiIconTypeImplementation final : public GuiGroupTypeImplementation {
public:
    GuiIconTypeImplementation(GuiWidgetOwner&, GuiIconRuntimeServices);
    void constructed74(GuiWidgetOwner&) override;
    void properties_bound(GuiWidgetOwner&, const GuiTable&) override;
    void loaded78(GuiWidgetOwner&) override;
    //00AB6120: supported non-delayed records have both tail flags clear.
    // A delayed record requiring a load/unload fails explicitly.
    void visibility_changed3c(GuiWidgetOwner&, bool) override;
    GuiIconRuntime& runtime() noexcept { return runtime_; }
private:
    GuiIconRuntime runtime_;
};
class GuiFrameBoxTypeImplementation final : public GuiGroupTypeImplementation {
public:
    GuiFrameBoxTypeImplementation(GuiWidgetOwner&, GuiFrameBoxRuntimeServices);
    void constructed74(GuiWidgetOwner&) override;
    void properties_bound(GuiWidgetOwner&, const GuiTable&) override;
    void loaded78(GuiWidgetOwner&) override;
    GuiFrameBoxWidget& state() noexcept { return state_; }
private:
    GuiFrameBoxWidget state_;
    GuiFrameBoxRuntimeServices services_;
};
class GuiScreenLayerImplementation final : public GuiGroupTypeImplementation {
public:
    GuiScreenLayerImplementation(GuiWidgetOwner&, std::uint8_t, GuiScreenLayerServices);
    void before_properties(GuiWidgetOwner&, const GuiTable&) override;
    void loaded78(GuiWidgetOwner&) override;
    bool is_visible38(GuiWidgetOwner&) override;
    void set_visible34(GuiWidgetOwner&, bool) override;
    void before_scene_release(GuiWidgetOwner&) override;
    GuiScreenLayerState& state() noexcept { return state_; }
private:
    GuiScreenLayerState state_;
    GuiScreenLayerServices services_;
    bool retired_{};
};
struct GuiTypeDispatchServices {
    // Resource callbacks must resolve the SAME owner's model/geometry identity.
    // The factory itself binds proven base loaded/position/recompose callbacks.
    std::function<GuiIconRuntimeServices(GuiWidgetOwner&)> icon;
    std::function<GuiFrameBoxRuntimeServices(GuiWidgetOwner&)> framebox;
    std::function<GuiScreenLayerServices(GuiWidgetOwner&)> screen;
};
class GuiTypeDispatchFactory {
public:
    explicit GuiTypeDispatchFactory(GuiTypeDispatchServices);
    // Call immediately BEFORE runtime.construct_root(root, actual_node), after
    // root.type/key are set. The exact per-page byte is consumed during make_type.
    // Plain/default00AA3840 and copy construction are intentionally unsupported:
    // their unwritten native preimage cannot be invented from the script path.
    void prepare_script_page(GuiLayoutWidget& root, std::uint8_t screen_flag);
    void cancel_script_page(GuiLayoutWidget& root) noexcept;
    GuiWidgetImplementationFactory make_factory() const;
private:
    struct Shared;
    std::shared_ptr<Shared> shared_;
};
} // namespace bsp
