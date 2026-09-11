#pragma once
#include "bsp/gui_render_order.hpp"
#include "bsp/native_camera_reference.hpp"
#include "bsp/system_lighting_owners.hpp"

namespace bsp {

// The outer cScene is 24h bytes; GuiLights is the DIFFERENT 3Ch resource
// referenced at +1C. Pointer fields use existing host hierarchy companions,
// as NativeNodeStorage does. This is a new C++ ABI, not a binary replacement.
struct NativeGuiSceneStorage {
    std::uint32_t vtable_00;
    std::atomic<std::int32_t> references_04;
    void* weak_handle_08;
    CameraTransform* first_root_0c;
    NativeString name_10;
    std::uint32_t scalar_18;
    SceneResource* lighting_1c;
    std::uint32_t field_20;
};

// Required real weak-handle owner/pool operations. Construction establishes
// +00/+04/+08 via 925490; destruction invalidates/releases that same handle
// under the actual pool lock via 925540, ending in CEB130 base-vtable phase.
// No default, fabricated handle, or independent scene count is supplied.
class NativeGuiSceneWeakBase {
public:
    virtual ~NativeGuiSceneWeakBase() = default;
    virtual void construct_00925490(NativeGuiSceneStorage&) = 0;
    virtual void destroy_00925540(NativeGuiSceneStorage&) noexcept = 0;
};
struct NativeGuiSceneEnvironment {
    NativeNodeDestructionRuntime& nodes;
    NativeGuiSceneWeakBase& weak_base;
    const volatile std::uint32_t& one_00d7a24c;
    const volatile std::uint32_t* vtable_00d62d48; // BD30E0, B72580, B73970
};

// Canonical companion borrowing actual +04 and +0C/+1C fields. Factory
// returns the initial native reference. Its zero callback destroys roots,
// frees native storage and deletes this companion; queued references delay it.
class NativeGuiSceneOwner final : public RenderCommandReference {
public:
    NativeGuiSceneStorage& storage;
    NativeGuiSceneEnvironment& environment;
    RenderNodeRootList roots;
    SystemSceneResourceSlot lighting;
    void release_zero_references() noexcept override;
    ~NativeGuiSceneOwner() override;
private:
    friend NativeGuiSceneOwner* allocate_native_gui_scene_00b724e0(
        NativeGuiSceneEnvironment&, const NativeString&);
    NativeGuiSceneOwner(NativeGuiSceneStorage&, NativeGuiSceneEnvironment&) noexcept;
    bool live_{};
};
NativeGuiSceneOwner* allocate_native_gui_scene_00b724e0(
    NativeGuiSceneEnvironment&, const NativeString&);
void release_native_gui_scene_00b72250(NativeGuiSceneOwner&) noexcept;
// Real B723F0 publication/retain/release on the outer scene's actual +1C.
void set_native_gui_scene_lighting_00b723f0(
    NativeGuiSceneOwner&, ConcreteSystemSceneResource*);
// Same node root chain used by scene destruction. This transfers no reference:
// creator/self lifetime is consumed later by the root's B6DFA0 -> virtual18.
void attach_native_gui_scene_node_00b6d890(
    NativeGuiSceneOwner&, GeneratedModelNodeLifetime&);

// Direct camera pool allocation, B71A80 and the existing canonical reference.
// The environment must survive all queued references. No synthetic camera.
NativeCameraReference* allocate_native_gui_camera(
    NativeCameraEnvironment&, const NativeString&);

// Store pointers borrow concrete native owner identities, never companions or
// another refcount. The caller retains its canonical owner bindings while any
// store/layer/queue can use them. There is only the supplied ordered map.
GuiCameraStore* create_gui_camera_store_00aa5070(GuiCameraStoreMap&,
    NativeCameraReference&, NativeGuiSceneOwner&, const GuiCameraStoreKey&);
// No release of camera or scene. Frees the first matching record, nulls its
// map value, then erases that entry. Missing records are a no-op.
bool remove_gui_camera_store_00aa4b30(GuiCameraStoreMap&, GuiCameraStore*) noexcept;
// Checked borrowed projections from the SAME store and existing companions;
// callers supply the canonical owner, not an invented owner for a raw pointer.
NativeGuiSceneOwner& require_gui_store_scene(const GuiCameraStore&, NativeGuiSceneOwner&);
NativeCameraReference& require_gui_store_camera(const GuiCameraStore&, NativeCameraReference&);

} // namespace bsp
