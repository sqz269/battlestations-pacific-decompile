#include "bsp/system_lighting_owners.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <cstring>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error System lighting owner reconstruction requires MSVC Win32 pointer widths.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(SystemAmbientBacklinks) == 12);
static_assert(offsetof(ConcreteSystemAmbientLight, references_04) == 4);
static_assert(offsetof(ConcreteSystemAmbientLight, scenes_08) == 8);
static_assert(offsetof(ConcreteSystemAmbientLight, ambient_mode3_28) == 0x28);
static_assert(offsetof(ConcreteSystemAmbientLight, ambient_cube_38) == 0x38);
static_assert(offsetof(ConcreteSystemAmbientLight, lighting) == 0x98);

namespace {
std::int32_t signed_word(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
}

void reserve_system_ambient_backlinks_00b7b390(
    SystemAmbientBacklinks& array, std::int32_t minimum) {
    if (minimum < 1) minimum = 1;
    if (minimum <= array.capacity_08) return;
    const auto bytes = static_cast<std::uint32_t>(minimum) * 4u;
    auto* allocated = static_cast<SceneResource**>(singleton_lifetime_allocate(
        {SingletonAllocationKind::pointer_slots, bytes, bytes}));
    for (std::int32_t i = 0; i < array.count_04; ++i) allocated[i] = array.begin_00[i];
    singleton_lifetime_free(array.begin_00);
    // 00B7B3E1..00B7B3E9 continues after the incorrectly no-return free call.
    array.begin_00 = allocated;
    array.capacity_08 = minimum;
}

bool erase_system_ambient_backlink_00b7b620(
    SystemAmbientBacklinks& array, SceneResource* const* key) {
    for (std::int32_t i = 0; i < array.count_04; ++i) {
        if (array.begin_00[i] != *key) continue;
        const auto last = array.count_04 - 1;
        if (i != last) array.begin_00[i] = array.begin_00[last];
        --array.count_04;
        return true;
    }
    return false;
}

void resize_system_ambient_backlinks_00b7bc70(
    SystemAmbientBacklinks& array, std::int32_t count) {
    if (array.capacity_08 < count) reserve_system_ambient_backlinks_00b7b390(array, count);
    for (auto i = array.count_04; i < count; ++i) array.begin_00[i] = nullptr;
    while (count < array.count_04) --array.count_04;
    array.count_04 = count;
}

ConcreteSystemAmbientLight::ConcreteSystemAmbientLight(
    const SystemLightingWords4& preimage) noexcept
    : vtable_00(0x00d62f3cu), references_04(1), scenes_08{nullptr, 0, 0},
      scalar_14(0x3f800000u), ambient_18{0, 0, 0, 0x3f800000u},
      ambient_mode3_28(preimage), lighting{ambient_18, ambient_mode3_28, ambient_cube_38} {
    for (auto& face : ambient_cube_38) face = {0, 0, 0, 0x3f800000u};
}

ConcreteSystemAmbientLight* construct_system_ambient_00b7c290(void* raw_owner) {
    SystemLightingWords4 preimage;
    std::memcpy(preimage.data(), static_cast<unsigned char*>(raw_owner) + 0x28, sizeof(preimage));
    return ::new (raw_owner) ConcreteSystemAmbientLight(preimage);
}

ConcreteSystemAmbientLight* allocate_system_ambient_00b7c290() {
    auto* raw = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x98, sizeof(ConcreteSystemAmbientLight)});
    return construct_system_ambient_00b7c290(raw);
}

bool remove_system_ambient_scene_00b7bd50(
    ConcreteSystemAmbientLight& ambient, SceneResource* scene) {
    return erase_system_ambient_backlink_00b7b620(ambient.scenes_08, &scene);
}

void append_system_ambient_scene_00b7bf90(
    ConcreteSystemAmbientLight& ambient, SceneResource* scene) {
    auto& array = ambient.scenes_08;
    if (array.count_04 == array.capacity_08) {
        auto capacity = signed_word(static_cast<std::uint32_t>(array.capacity_08) * 2u);
        if (capacity < 2) capacity = 1;
        reserve_system_ambient_backlinks_00b7b390(array, capacity);
    }
    array.begin_00[array.count_04] = scene;
    array.count_04 = signed_word(static_cast<std::uint32_t>(array.count_04) + 1u);
}

void destroy_system_ambient_00b7c450(ConcreteSystemAmbientLight& ambient) noexcept {
    resize_system_ambient_backlinks_00b7bc70(ambient.scenes_08, 0);
    singleton_lifetime_free(ambient.scenes_08.begin_00);
    // 00B7C48A..00B7C4B2: free returns; native leaves begin/capacity unchanged.
    ambient.vtable_00 = 0x00d5c104u;
    ambient.vtable_00 = 0x00ceb130u; // actual 00BD30F0 body
}

ConcreteSystemAmbientLight* delete_system_ambient_00b7c7e0(
    ConcreteSystemAmbientLight* ambient, std::uint32_t flags) noexcept {
    auto* const original = ambient;
    destroy_system_ambient_00b7c450(*ambient);
    if (flags & 1u) {
        ambient->~ConcreteSystemAmbientLight();
        singleton_lifetime_free(ambient);
    }
    return original;
}

void release_system_ambient(ConcreteSystemAmbientLight& ambient) noexcept {
    if (ambient.references_04.fetch_sub(1) == 1)
        delete_system_ambient_00b7c7e0(&ambient, 1);
}

SystemSceneResourceLightList::SystemSceneResourceLightList(
    SceneNodeRegistry& registry, SystemDirectionalLightResolver& resolver) noexcept
    : registry_(registry), resolver_(resolver) {}
const void* SystemSceneResourceLightList::sentinel_1c() { return registry_.native_sentinel(); }
const void* SystemSceneResourceLightList::next_00(const void* node) {
    return SceneNodeRegistry::native_next(node);
}
SystemDirectionalLight* SystemSceneResourceLightList::light_08(const void* node) {
    return resolver_.resolve_light(SceneNodeRegistry::native_key(node));
}

SystemSceneResourceLighting::SystemSceneResourceLighting(
    ConcreteSystemSceneResource& owner) noexcept : owner_(owner) {}
SystemLightEnvironment* SystemSceneResourceLighting::environment_10() const {
    return owner_.environment_10 ? &owner_.environment_10->lighting : nullptr;
}
SystemLightListAccess& SystemSceneResourceLighting::light_list() const { return owner_.light_list; }

namespace {
void destroy_scene_resource_zero(SceneResource& base) {
    // This is the actual native virtual+00 policy for this concrete type:
    // 00BD30E0 ->00B83410(flag1), with no arbitrary user destruction callback.
    delete_system_scene_resource_00b83410(static_cast<ConcreteSystemSceneResource*>(&base), 1);
}
}

ConcreteSystemSceneResource::ConcreteSystemSceneResource(SizedStoragePool& pool,
    const NativeString& name, SystemDirectionalLightResolver& resolver)
    : SceneResource(1, destroy_scene_resource_zero, this, SceneRegistryInitialization::deferred),
      vtable_00(0x00d63168u),
      light_list(registry, resolver), lighting(*this), string_pool(pool) {
    try {
        PooledStringStorage strings(pool);
        name_08.copy_from_00be0a30_fragment(strings, name);
        environment_10 = nullptr;
        registry.initialize_native_00b83600();
        auto* ambient = allocate_system_ambient_00b7c290();
        set_system_scene_environment_00b825d0(*this, ambient);
        release_system_ambient(*ambient); // creator's reference, after setter retain
    } catch (...) {
        // CC23B9 unwind states: registry, name, root base. The native raw ambient
        // slot has no unwind destructor: an exception in backlink append is not
        // converted here into a different ambient retain/release transaction.
        registry.destroy_native_bucket_storage_00b82ed0();
        registry.destroy_native_list_storage_00b829d0();
        if (name_08.data()) pool.release_00bd1510(name_08.data(), name_08.length() + 1u);
        vtable_00 = 0x00ceb130u;
        throw;
    }
}

ConcreteSystemSceneResource* allocate_system_scene_resource_00b83c50(
    SizedStoragePool& pool, const NativeString& name, SystemDirectionalLightResolver& resolver) {
    auto* raw = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x3c, sizeof(ConcreteSystemSceneResource)});
    try {
        return ::new (raw) ConcreteSystemSceneResource(pool, name, resolver);
    } catch (...) {
        singleton_lifetime_free(raw);
        throw;
    }
}

void set_system_scene_environment_00b825d0(
    ConcreteSystemSceneResource& scene, ConcreteSystemAmbientLight* requested) {
    if (scene.environment_10) remove_system_ambient_scene_00b7bd50(*scene.environment_10, &scene);
    auto* previous = scene.environment_10;
    if (previous != requested) {
        scene.environment_10 = requested;
        if (requested) requested->references_04.fetch_add(1);
        if (previous) release_system_ambient(*previous);
    }
    if (scene.environment_10) append_system_ambient_scene_00b7bf90(*scene.environment_10, &scene);
}

void destroy_system_scene_resource_00b82ed0(ConcreteSystemSceneResource& scene) noexcept {
    scene.vtable_00 = 0x00d63168u;
    if (scene.environment_10) {
        // Native destructor releases directly. It does not remove the borrowed
        // scene backlink from an environment that survives due to another owner.
        release_system_ambient(*scene.environment_10);
        scene.environment_10 = nullptr;
    }
    scene.registry.destroy_native_bucket_storage_00b82ed0();
    scene.registry.destroy_native_list_storage_00b829d0();
    if (scene.name_08.data())
        scene.string_pool.release_00bd1510(scene.name_08.data(), scene.name_08.length() + 1u);
    // Native name length/pointer are not cleared by this inlined release.
    scene.vtable_00 = 0x00ceb130u;
}

ConcreteSystemSceneResource* delete_system_scene_resource_00b83410(
    ConcreteSystemSceneResource* scene, std::uint32_t flags) noexcept {
    auto* const original = scene;
    destroy_system_scene_resource_00b82ed0(*scene);
    if (flags & 1u) {
        scene->~ConcreteSystemSceneResource();
        singleton_lifetime_free(scene);
    }
    return original;
}

void set_system_scene_resource_00b723f0(SceneResource*& actual_slot, SceneResource* requested) {
    auto* previous = actual_slot;
    if (previous == requested) return;
    actual_slot = requested;
    if (requested) requested->references.fetch_add(1);
    if (previous && previous->references.fetch_sub(1) == 1) previous->destroy_on_zero(*previous);
}

SystemSceneResourceSlot::SystemSceneResourceSlot(SceneResource* const& slot) noexcept : actual_slot_(slot) {}
SystemSceneLighting* SystemSceneResourceSlot::lighting_1c() const {
    auto* resource = actual_slot_;
    if (!resource) return nullptr;
    if (resource->destroy_on_zero != destroy_scene_resource_zero || resource->context != resource)
        throw std::logic_error("outer scene+1C has no concrete system scene-resource owner binding");
    return &static_cast<ConcreteSystemSceneResource*>(resource)->lighting;
}

} // namespace bsp
