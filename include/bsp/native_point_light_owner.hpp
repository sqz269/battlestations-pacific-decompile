#pragma once
#include "bsp/directional_light_owner.hpp"
#include "bsp/light_type_bootstrap.hpp"
#include "bsp/native_point_light_pool.hpp"

namespace bsp {
struct NativePointLightTypeDescriptor {
    std::uint32_t own_id, light_id, node_id, root_id, native_name_address;
};
struct NativePointLightTypeStorage {
    volatile std::uint8_t& guard_0109010f;
    volatile NativePointLightTypeDescriptor& point_010901b0;
};
// Same actual monotonic counter and inherited descriptors as every other type.
// Initialization is explicit; guards and descriptor words are never defaulted.
class NativePointLightTypes final {
public:
    NativePointLightTypes(TypeIdCounterLifetime&, LightTypeBootstrap&,
        NativePointLightTypeStorage) noexcept;
    void initialize_00cd81a0();
    bool is_type_00b7c740(std::uint32_t) const noexcept;
    NativePointLightTypeStorage storage() const noexcept { return storage_; }
private:
    std::uint32_t consume_id();
    TypeIdCounterLifetime& counter_;
    LightTypeBootstrap& shared_;
    NativePointLightTypeStorage storage_;
};

// Actual +174..+1EB tail. This is a distinct lifetime from the directional
// layout: +1E0 is the physical backlink descriptor, never a direction view.
// Native leaves +1EC..+1FB unwritten; the pool owns the live +1FC slab ID.
struct NativePointLightTailStorage {
    void* shadow_174;
    SystemAmbientBacklinks scenes_178;
    SystemLightingWords4 diffuse_184, specular_194, base_diffuse_1a4;
    SystemLightingWords4 diffuse_mode3_1b4, base_specular_1c4;
    std::uint32_t scalar_1d4, diffuse_scale_1d8, specular_scale_1dc;
    NativePointLightBacklinkArray backlinks_1e0;
};
struct NativePointLightStorageView {
    NativeNodeStorage& node;
    NativePointLightTailStorage& light;
};
// ECX actual 200h slot, name on stack, EAX same slot, RET4. Reuses the complete
// Light constructor, ends its temporary tail view, and initializes actual1E0.
NativePointLightStorageView construct_native_point_light_00b7c710(void*,
    std::size_t actual_slot_bytes, const NativeString&, SizedStoragePool&);

struct NativePointLightProfiles {
    const volatile std::uint32_t* point_00d63008;
    const volatile std::uint32_t* light_00d62f58;
    const volatile std::uint32_t* node_00d62c88;
};
struct NativePointLightEnvironment {
    NativePointLightPool& pool_0109011c;
    NativeNodeDestructionRuntime& nodes;
    SceneTypePredicate point_virtual_0c; // actual B7C740, initialized descriptor
    SceneTypePredicate light_virtual_0c; // actual B7C580, same inherited tokens
    NativePointLightProfiles profiles;   // actual immutable 22-DWORD tables
};

// Stable companion; no second hierarchy, scene array, light array or +04 count.
// Construct after raw construction. Explicitly bind node.scene_attachment and
// backlinks in the SAME environment runtimes before exposure/destruction.
class NativePointLightOwner final {
public:
    NativePointLightOwner(NativePointLightStorageView, NativePointLightEnvironment&);
    ~NativePointLightOwner() = default; // explicit native destruction only
    NativePointLightOwner(const NativePointLightOwner&) = delete;
    NativePointLightOwner& operator=(const NativePointLightOwner&) = delete;
    enum class Phase { live, destroying, dead };
    NativePointLightEnvironment& environment;
    NativePointLightTailStorage& light;
    NativeNodeBinding node;
    LightSceneRetention retained_scenes;
    NativePointLightLinksBinding backlinks;
    Phase phase{Phase::live}; // host lifetime bookkeeping only
};
// Complete B7C770, ECX light, RET. Point -> Light -> Node; unlinks/frees the
// actual1E0 array, unbinds that descriptor before its lifetime ends, then uses
// the existing complete Light body/unwind. Does not return the slot.
void destroy_native_point_light_00b7c770(NativePointLightOwner&);
// B7C850, ECX light, stack flags, EAX original address, RET4. flags&1 returns
// the SAME200h slot; flags0 preserves destroyed backing for explicit pool return.
void* delete_native_point_light_00b7c850(NativePointLightOwner&, std::uint32_t flags);

class NativePointLightReference;
struct NativePointLightCompanionDisposal {
    void* context;
    void (*retire)(void*, NativePointLightReference&) noexcept;
};
class NativePointLightReference final : public GeneratedModelNodeLifetime,
    public RenderCommandReference {
public:
    NativePointLightReference(NativePointLightOwner&, NativePointLightCompanionDisposal);
    ~NativePointLightReference() override;
    NativePointLightReference(const NativePointLightReference&) = delete;
    NativePointLightReference& operator=(const NativePointLightReference&) = delete;
    NativePointLightOwner& light_owner() noexcept { return owner_; }
    CameraTransform& transform() noexcept override { return owner_.node.transform; }
    SceneNodeAttachment& scene_attachment() noexcept override { return owner_.node.scene_attachment; }
    void release_model_virtual18_00b6f310() noexcept override;
    void remove_scene_virtual54(SceneResource*, bool recurse) noexcept override;
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    NativePointLightOwner& owner_;
    GeneratedModelLifetimeRuntime& runtime_;
    NativePointLightCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    const volatile std::uint32_t* current_table() const noexcept;
    static std::uint32_t light_count(void*) noexcept;
    static void remove_light_backlink(void*, std::uint32_t, CameraTransform&) noexcept;
    static void shrink_lights(void*) noexcept;
};
// New C++ owning composition: actual pool -> ctor -> scene/backlink/lifetime
// binding. Owns the native self reference, with terminal BD30E0 -> B7C850(1).
// Host setup failure reclaims only this unpublished fresh construction.
NativePointLightReference* allocate_native_point_light(NativePointLightEnvironment&,
    const NativeString&);
// Host ownership composition after the caller has performed the actual raw
// allocation and B7C710 construction in its native order. Requires a fresh,
// unbound slot from this SAME environment. Consumes that constructed slot;
// host companion/binding failure destroys it and returns it to the same pool.
// This adds no native allocation, constructor, retain, or alternate owner graph.
NativePointLightReference* adopt_constructed_native_point_light(
    NativePointLightEnvironment&, NativePointLightStorageView);
} // namespace bsp
