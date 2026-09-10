#pragma once
#include "bsp/native_string.hpp"
#include "bsp/scene_attachment.hpp"
#include "bsp/system_lighting_constants.hpp"
#include <atomic>
#include <cstdint>

namespace bsp {

// Native pointer/count/capacity array at ambient+08. Scene pointers are borrowed;
// destroying or erasing an entry never changes a SceneResource reference count.
struct SystemAmbientBacklinks {
    SceneResource** begin_00;
    std::int32_t count_04;
    std::int32_t capacity_08;
};

void reserve_system_ambient_backlinks_00b7b390(SystemAmbientBacklinks&, std::int32_t minimum);
bool erase_system_ambient_backlink_00b7b620(SystemAmbientBacklinks&, SceneResource* const* key);
void resize_system_ambient_backlinks_00b7bc70(SystemAmbientBacklinks&, std::int32_t count);

// Native field layout through+98 on MSVC Win32, followed by a borrowed host view.
// The vtable DWORD records native transitions; it is not a callable host vtable.
// C++ lifetime is explicit: body destruction leaves the native dangling vector
// pointer/capacity, while the scalar destructor optionally frees the allocation.
struct ConcreteSystemAmbientLight {
    std::uint32_t vtable_00;
    std::atomic<std::int32_t> references_04;
    SystemAmbientBacklinks scenes_08;
    std::uint32_t scalar_14;
    SystemLightingWords4 ambient_18;
    SystemLightingWords4 ambient_mode3_28;
    std::array<SystemLightingWords4, 6> ambient_cube_38;
    SystemLightEnvironment lighting;

    explicit ConcreteSystemAmbientLight(const SystemLightingWords4& allocation_preimage) noexcept;
    ConcreteSystemAmbientLight(const ConcreteSystemAmbientLight&) = delete;
    ConcreteSystemAmbientLight& operator=(const ConcreteSystemAmbientLight&) = delete;
};

// raw_owner must have sizeof(ConcreteSystemAmbientLight) raw bytes. Reads its
// actual native+28..37 bytes before placement construction; no zero substitute.
ConcreteSystemAmbientLight* construct_system_ambient_00b7c290(void* raw_owner);
ConcreteSystemAmbientLight* allocate_system_ambient_00b7c290();
bool remove_system_ambient_scene_00b7bd50(ConcreteSystemAmbientLight&, SceneResource*);
void append_system_ambient_scene_00b7bf90(ConcreteSystemAmbientLight&, SceneResource*);
void destroy_system_ambient_00b7c450(ConcreteSystemAmbientLight&) noexcept;
ConcreteSystemAmbientLight* delete_system_ambient_00b7c7e0(
    ConcreteSystemAmbientLight*, std::uint32_t flags) noexcept;
// Native virtual+00 ->00BD30E0 invokes the actual scalar destructor with flag1.
void release_system_ambient(ConcreteSystemAmbientLight&) noexcept;

struct ConcreteSystemSceneResource;

// Reads the one canonical SceneNodeRegistry. The resolver associates actual
// native pointer keys with live light views, including a nonmember sentinel key.
class SystemSceneResourceLightList final : public SystemLightListAccess {
public:
    SystemSceneResourceLightList(SceneNodeRegistry&, SystemDirectionalLightResolver&) noexcept;
    const void* sentinel_1c() override;
    const void* next_00(const void* node) override;
    SystemDirectionalLight* light_08(const void* node) override;
private:
    SceneNodeRegistry& registry_;
    SystemDirectionalLightResolver& resolver_;
};

class SystemSceneResourceLighting final : public SystemSceneLighting {
public:
    explicit SystemSceneResourceLighting(ConcreteSystemSceneResource&) noexcept;
    SystemLightEnvironment* environment_10() const override;
    SystemLightListAccess& light_list() const override;
private:
    ConcreteSystemSceneResource& owner_;
};

// Extends the same SceneResource used by SceneAttachmentRuntime. New C++ ABI,
// not a native+3C overlay. Name/ambient own storage; registry borrows light keys.
// The pool and resolver must outlive this owner. Destruction remains explicit.
struct ConcreteSystemSceneResource final : SceneResource {
    ConcreteSystemSceneResource(SizedStoragePool&, const NativeString& name,
        SystemDirectionalLightResolver&);
    std::uint32_t vtable_00;
    NativeString name_08;
    ConcreteSystemAmbientLight* environment_10;
    SystemSceneResourceLightList light_list;
    SystemSceneResourceLighting lighting;
    SizedStoragePool& string_pool;
};

ConcreteSystemSceneResource* allocate_system_scene_resource_00b83c50(
    SizedStoragePool&, const NativeString& name, SystemDirectionalLightResolver&);
void set_system_scene_environment_00b825d0(
    ConcreteSystemSceneResource&, ConcreteSystemAmbientLight*);
void destroy_system_scene_resource_00b82ed0(ConcreteSystemSceneResource&) noexcept;
ConcreteSystemSceneResource* delete_system_scene_resource_00b83410(
    ConcreteSystemSceneResource*, std::uint32_t flags) noexcept;

// The slot belongs to an already supplied actual outer owner (native+1C).
// This does not stand up the unresolved weak-handle base or world owner.
void set_system_scene_resource_00b723f0(SceneResource*& actual_slot, SceneResource* requested);
class SystemSceneResourceSlot final : public SystemLightingScene {
public:
    explicit SystemSceneResourceSlot(SceneResource* const& actual_slot) noexcept;
    SystemSceneLighting* lighting_1c() const override;
private:
    SceneResource* const& actual_slot_;
};

} // namespace bsp
