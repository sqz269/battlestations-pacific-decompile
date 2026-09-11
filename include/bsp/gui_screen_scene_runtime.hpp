#pragma once
#include "bsp/directional_light_owner.hpp"
#include "bsp/gui_camera_store_owner.hpp"
#include "bsp/gui_type_dispatch.hpp"

namespace bsp {
// Both views must refer to ONE fresh B7C6B0 object in the supplied node runtime.
// The required lifetime implements its actual virtual18/54/00 and final pool
// return; the screen neither invents a light reference nor owns its companion.
struct GuiScreenDirectionalAllocation {
    DirectionalLightOwner& owner;
    GeneratedModelNodeLifetime& lifetime;
};
class GuiScreenSceneNativeCalls {
public:
    virtual ~GuiScreenSceneNativeCalls() = default;
    virtual GuiScreenDirectionalAllocation allocate_directional_light_00b7c6b0(
        const NativeString& name) = 0;
    // Actual CRT BF7030(ST0 double), float spill/reload, BF7420(ST0 -> EAX),
    // CVTSI2SS. BF7420 is a truncating conversion, NOT a clock. Preserve CRT/FPU
    // effects. D7A308 contains double2.0 in the inspected executable.
    virtual float root_radius_00ac5f00(const volatile double& squared_radius) = 0;
    // SAME actual cGroup root: clear flags138 bits30, byte175=0, write08..17.
    // A NativeModelOwner tail is not interchangeable with that group tail.
    virtual void set_root_bounds_00b8e6c0(NativeNodeBinding&,
        const SystemLightingWords4&) = 0;
    virtual void register_page_00aa52a0(GuiWidgetOwner&, GuiScreenLayerState&) = 0;
    virtual bool visibility_hook_installed_00f8bf4c() = 0;
    virtual void visibility_hook_00f8bf4c(std::string_view, bool) = 0;
};
struct GuiScreenSceneEnvironment {
    GuiCameraStoreMap& stores;
    NativeGuiSceneEnvironment& scenes;
    NativeCameraEnvironment& cameras;
    SystemDirectionalLightResolver& directional_lights;
    GuiScreenSceneNativeCalls& native;
    const volatile std::uint32_t& half_00ce3800;
    const volatile std::uint32_t& one_00d7a24c;
    const volatile double& squared_radius_00d7a308;
    const volatile std::uint32_t* directional_vtable_00d62fb0;
};

// Supplies GuiTypeDispatchServices::screen. State remains in the SAME
// GuiScreenLayerImplementation, and base node/tree state in GuiWidgetOwner.
// Only borrowed owner associations are stored here; the single ordered map and
// actual native owners supply all fields/counts/hierarchy. Services retain this
// adapter's association state; every supplied environment must outlive them and
// all native/queued references. No renderer or orthographic pass is fabricated.
class GuiScreenSceneRuntime final {
public:
    explicit GuiScreenSceneRuntime(GuiScreenSceneEnvironment);
    GuiScreenLayerServices make_services(GuiWidgetOwner&);
    // For an already-present store constructed by the canonical owner API.
    // This registers identities only and takes no native reference.
    void bind_existing_store(GuiCameraStore&, NativeCameraReference&, NativeGuiSceneOwner&);
    // Borrow these SAME references for the later orthographic/queue adapter.
    // Store removal invalidates lookup; queue retention remains explicit.
    NativeCameraReference& camera_for_store(GuiCameraStore&);
    NativeGuiSceneOwner& scene_for_store(GuiCameraStore&);
private:
    struct Shared;
    std::shared_ptr<Shared> shared_;
};
} // namespace bsp
