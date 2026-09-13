#pragma once
#include "bsp/native_unit_scene_handle.hpp"
#include "bsp/native_node_destruction.hpp"
#include "bsp/pose_refresh.hpp"

namespace bsp {

// Borrowed fields of the SAME canonical unit; no native layout cast, new unit,
// model, property bag, scene owner or default matrix. Binding/lifetimes must be
// established by the owning runtime. Current GameUnitsHost cannot provide them.
struct NativeUnitSceneInitializationView {
    NativeUnitSceneHandleView scene;
    PoseRefreshView& pose;
    void*& model_360;
    void*& property_holder_c0;
    void*& descriptor_538;
    void*& registration_168;
    void*& cloned_properties_724;
    float& shadow_factor_52c;
    std::uint8_t& byte_62c;
};
// The older scene_entity_create.hpp refs label is corrected by the native
// 00922DE0 kind dispatch and CRUISE_SPEED_SETTING.md: +04 is the kind, and
// +08 is a property bag only for kind 1. No refcount operation is inferred.
struct NativeUnitScenePropertyHolderView {
    const std::int32_t& kind_04;
    void* const& bag_08;
};
// Actual property record, not semantic ScenePropertyValue storage. payload_0c
// borrows at least four live bytes; reflection uses its first byte, numeric
// access uses all four. Existing ScenePropertyType::Float is native value 1.
struct NativeUnitScenePropertyView {
    const std::int32_t& type_04;
    const void* payload_0c;
};
struct NativeUnitSceneInitializationGlobals {
    void* const& world_00e188a8;
    void* const& manager_00e1aea0;
};

class NativeUnitSceneInitializationBindings {
public:
    explicit NativeUnitSceneInitializationBindings(NativeNodeDestructionRuntime& nodes) noexcept
        : nodes_(nodes) {}
    virtual ~NativeUnitSceneInitializationBindings() = default;
    // Pure borrowed field/identity access: no mutations, callbacks, allocation,
    // caching, substituted owners or FP-environment changes. Invalid required
    // identities have no fabricated null fallback (native dereferences them).
    virtual void* model_inner_160(void* actual_model) = 0;
    virtual void* inner_handle_0c(void* actual_inner) = 0;
    virtual void* world_root_19ec(void* actual_world) = 0;
    virtual std::uint32_t descriptor_class_70(void* actual_descriptor) = 0;
    virtual NativeUnitScenePropertyHolderView property_holder(void*) = 0;
    virtual NativeUnitScenePropertyView property_record(void*) = 0;
    virtual CameraTransform& node_transform(void* actual_node) = 0;
    virtual RenderNodeRootList* root_list(void* actual_root) = 0;

    // Complete external providers, read before contracting; see the doc table.
    // No exception suppression or noexcept narrowing. No automatic unwind guard
    // is inserted by this caller; native callee EH equivalence remains unproved.
    virtual void call_0087bcc0(void* canonical_unit) = 0;
    // Select current actual node table+30 and invoke it with the LIVE three
    // floats at unit+FC (world_cc[12..14]), not a copied/full matrix. D62DE8's
    // slot is B6DAE0, whose x87 setter tail-dispatches current virtual+34.
    virtual void node_position_30(void* actual_node, const float* actual_xyz) = 0;
    virtual std::uint32_t unit_primary_10(void* canonical_unit) = 0;
    virtual void call_009292b0(void* canonical_unit, std::uint32_t class_index,
        std::uint32_t primary_10_result) = 0;
    virtual void* call_00740fe0(void* actual_manager) = 0;
    virtual void* find_property_008f2260(void* actual_bag, const char* key) = 0;
    virtual void* clone_bag_008f41f0(void* actual_bag) = 0;
    virtual void clear_node_mask_00728340(void* actual_node, std::uint32_t mask) = 0;
    virtual void set_node_mask_00475e60(void* actual_node, std::uint32_t mask) = 0;
    // Native ECX=node, EDX=&actual unit+52C, bare RET: whole recursive material
    // operation, including cShadowFactor registration and diffuse alpha writes.
    virtual void call_00747560(void* actual_node, const float* actual_factor) = 0;
    // Default delegates to the existing complete canonical provider on the
    // resolved SAME node/root storage. Never supplies private list surgery.
    virtual void propagate_root_00b6d890(void* actual_node, void* actual_root);
private:
    NativeNodeDestructionRuntime& nodes_;
};

// Complete normal caller 00955420..0095559E (383 bytes). ECX=unit, bare RET;
// new C++ ABI. SSE conversion follows current MXCSR rounding, float payloads
// retain bits. Does not establish provider availability, native EH, complete
// object construction, runtime scene binding or gameplay equivalence.
void initialize_native_unit_scene_00955420(NativeUnitSceneInitializationView,
    NativeUnitSceneInitializationGlobals, NativeUnitSceneInitializationBindings&);

} // namespace bsp
