#pragma once
#include "bsp/directional_light_pool.hpp"
#include "bsp/light_scene_retention.hpp"
#include "bsp/native_node_destruction.hpp"

namespace bsp {

// Actual 0x78-byte light tail at slot+174..1EB. It excludes the live pool slab
// ID at +1EC. Names describe current consumers; unknown fields stay raw words.
struct NativeLightTailStorage {
    void* shadow_174;
    SystemAmbientBacklinks scenes_178;
    SystemLightingWords4 diffuse_184;
    SystemLightingWords4 specular_194; // constructor preserves allocation bytes
    SystemLightingWords4 base_diffuse_1a4;
    SystemLightingWords4 diffuse_mode3_1b4; // constructor preserves allocation bytes
    SystemLightingWords4 base_specular_1c4;
    std::uint32_t scalar_1d4;
    std::uint32_t diffuse_scale_1d8;
    std::uint32_t specular_scale_1dc; // constructor preserves allocation bytes
    SystemLightingWords3 direction_1e0; // constructor preserves allocation bytes
};
struct NativeLightStorageView {
    NativeNodeStorage& node;
    NativeLightTailStorage& light;
};

// Native ECX actual owner, stack name, EAX owner, RET4. The caller supplies an
// actual 0x1F0 slot and string pool. These return reference views, allocate no
// slot/companion/shadow, and preserve every constructor-unwritten byte and +1EC.
NativeLightStorageView construct_native_light_00b7c4c0(void* actual_slot,
    std::size_t slot_bytes, const NativeString&, SizedStoragePool&);
NativeLightStorageView construct_native_directional_light_00b7c6b0(void* actual_slot,
    std::size_t slot_bytes, const NativeString&, SizedStoragePool&);

// Stable external companion. Every rendering field is a reference to the same
// raw tail used by destruction and LightSceneRetention. The shadow resolver
// maps a nonnull actual +174 identity to its real borrowed rendering view;
// creating any shadow owner is a separate contract. The native constructor's
// null +174 is supported directly without a fake owner or resolver result.
// Requires the fresh directional constructor result. Construction installs
// dispatch associations only. The caller explicitly
// binds node.scene_attachment in runtime.scenes before exposing the owner.
class DirectionalLightOwner final {
public:
    DirectionalLightOwner(NativeLightStorageView, DirectionalLightPool& actual_pool,
        NativeNodeDestructionRuntime&, SceneTypePredicate actual_directional_virtual_0c,
        SceneTypePredicate actual_light_virtual_0c, SystemShadowOwnerResolver&);
    DirectionalLightOwner(const DirectionalLightOwner&) = delete;
    DirectionalLightOwner& operator=(const DirectionalLightOwner&) = delete;
    ~DirectionalLightOwner() = default; // native destruction is explicit

    DirectionalLightPool& pool;
    NativeNodeDestructionRuntime& runtime;
    NativeLightTailStorage& light;
    NativeNodeBinding node;
    LightSceneRetention retained_scenes;
    SystemDirectionalLight lighting;
    const SceneTypePredicate directional_virtual_0c;
    const SceneTypePredicate light_virtual_0c;
};

// Complete native [B7C5B0,B7C6AA), ECX light, RET. Light phase drains the live
// back array with native callback reloads, releases actual shadow ownership,
// frees +178, then executes the direct node base destructor. It ends both raw
// lifetimes and forgets only the dead host scene association. It returns no slot.
void destroy_native_light_00b7c5b0(DirectionalLightOwner&);
// Native ECX directional owner, stack deletion flags, EAX original owner,
// RET4. Directional -> light -> node phases; returns the SAME pool slot only
// when flags&1. Returned raw pointer may already be free, as in the native ABI.
void* delete_native_directional_light_00b7c820(DirectionalLightOwner&, std::uint32_t flags);

} // namespace bsp
