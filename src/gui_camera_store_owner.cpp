#include "bsp/gui_camera_store_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <memory>
#include <new>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(NativeGuiSceneStorage) == 0x24);
static_assert(offsetof(NativeGuiSceneStorage, references_04) == 4);
static_assert(offsetof(NativeGuiSceneStorage, first_root_0c) == 0x0c);
static_assert(offsetof(NativeGuiSceneStorage, name_10) == 0x10);
static_assert(offsetof(NativeGuiSceneStorage, lighting_1c) == 0x1c);
static_assert(sizeof(GuiCameraStore) == 0x24);
static_assert(offsetof(GuiCameraStore, camera) == 0x18);
static_assert(offsetof(GuiCameraStore, scene) == 0x1c);
namespace {
void require_scene_profile(const NativeGuiSceneEnvironment& environment) {
    const auto* table = environment.vtable_00d62d48;
    if (!table || table[0] != 0x00bd30e0u || table[1] != 0x00b72580u)
        throw std::invalid_argument("GUI scene requires actual D62D48 owner profile");
}
void retire_camera(void*, NativeCameraReference& reference) noexcept {
    auto* owner = &reference.camera_owner();
    delete &reference;
    delete owner;
}
}
NativeGuiSceneOwner::NativeGuiSceneOwner(NativeGuiSceneStorage& actual,
    NativeGuiSceneEnvironment& access) noexcept
    : RenderCommandReference(actual.references_04), storage(actual), environment(access),
      roots(actual.first_root_0c, actual.lighting_1c), lighting(actual.lighting_1c) {}
NativeGuiSceneOwner::~NativeGuiSceneOwner() {
    if (live_) std::terminate();
}
NativeGuiSceneOwner* allocate_native_gui_scene_00b724e0(
    NativeGuiSceneEnvironment& environment, const NativeString& name) {
    require_scene_profile(environment);
    auto& name_pool = environment.nodes.require_semantic_name_pool();
    void* raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x24, 0x24});
    if (!raw) throw std::bad_alloc();
    // Establish typed lifetime but restore constructor preimage before the
    // weak-base callback can inspect bytes B724E0 has not initialized yet.
    std::byte preimage[0x24];
    std::memcpy(preimage, raw, sizeof(preimage));
    auto* actual = new (raw) NativeGuiSceneStorage;
    std::memcpy(raw, preimage, sizeof(preimage));
    NativeGuiSceneOwner* owner{};
    bool base_constructed = false;
    bool name_initialized = false;
    try {
        owner = new NativeGuiSceneOwner(*actual, environment);
        environment.weak_base.construct_00925490(*actual);
        base_constructed = true;
        actual->vtable_00 = 0x00d62d48u;
        actual->first_root_0c = nullptr;
        new (&actual->name_10) NativeString;
        name_initialized = true;
        actual->lighting_1c = nullptr;
        actual->field_20 = 0;
        PooledStringStorage strings(name_pool);
        actual->name_10.copy_from_00be0a30_fragment(strings, name);
        actual->scalar_18 = environment.one_00d7a24c;
        owner->live_ = true;
        return owner;
    } catch (...) {
        if (name_initialized) {
            PooledStringStorage strings(name_pool);
            destroy_native_string_header_0041dd20(&actual->name_10, strings);
        }
        if (base_constructed) environment.weak_base.destroy_00925540(*actual);
        delete owner;
        actual->~NativeGuiSceneStorage();
        singleton_lifetime_free(raw);
        throw;
    }
}
void NativeGuiSceneOwner::release_zero_references() noexcept {
    if (!live_ || storage.vtable_00 != 0x00d62d48u) std::terminate();
    try { require_scene_profile(environment); } catch (...) { std::terminate(); }
    SizedStoragePool* name_pool{};
    try { name_pool = &environment.nodes.require_semantic_name_pool(); } catch (...) { std::terminate(); }
    // B72430: release captured lighting, then clear the slot even if a callback
    // changed it. Reload root head after every potentially terminal node call.
    storage.vtable_00 = 0x00d62d48u;
    if (auto* captured = storage.lighting_1c) {
        if (captured->references.fetch_sub(1) == 1) captured->destroy_on_zero(*captured);
        storage.lighting_1c = nullptr;
    }
    while (roots.first)
        unlink_and_release_render_model_00b6dfa0(environment.nodes.attachments.resolve(*roots.first));
    PooledStringStorage strings(*name_pool);
    destroy_native_string_header_0041dd20(&storage.name_10, strings);
    environment.weak_base.destroy_00925540(storage);
    auto* raw = &storage;
    live_ = false;
    raw->~NativeGuiSceneStorage();
    singleton_lifetime_free(raw); // B72580 bit1; native returns freed identity
    delete this;
}
void release_native_gui_scene_00b72250(NativeGuiSceneOwner& scene) noexcept {
    release_render_command_reference(scene);
}
void set_native_gui_scene_lighting_00b723f0(
    NativeGuiSceneOwner& scene, ConcreteSystemSceneResource* lighting) {
    set_system_scene_resource_00b723f0(scene.storage.lighting_1c, lighting);
}
void attach_native_gui_scene_node_00b6d890(
    NativeGuiSceneOwner& scene, GeneratedModelNodeLifetime& node) {
    if (&scene.environment.nodes.attachments.resolve(node.transform()) != &node)
        throw std::invalid_argument("GUI scene node requires its canonical lifetime binding");
    propagate_native_node_root_00b6d890(scene.environment.nodes, node.transform(), &scene.roots);
}
NativeCameraReference* allocate_native_gui_camera(
    NativeCameraEnvironment& environment, const NativeString& name) {
    void* raw = environment.pool_0108ffb0.allocate_raw_slot_00b71770();
    if (!raw) throw std::bad_alloc();
    NativeCameraOwner* owner{};
    try {
        owner = new NativeCameraOwner(raw, NativeCameraPool::slot_bytes, environment);
        construct_native_camera_00b71a80(*owner, name);
        return new NativeCameraReference(*owner, {nullptr, retire_camera});
    } catch (...) {
        if (owner && owner->phase == NativeCameraOwner::Phase::live)
            delete_native_camera_00b71fe0(*owner, 0);
        delete owner;
        environment.pool_0108ffb0.return_raw_slot_00b711e0(raw);
        throw;
    }
}
GuiCameraStore* create_gui_camera_store_00aa5070(GuiCameraStoreMap& stores,
    NativeCameraReference& camera, NativeGuiSceneOwner& scene, const GuiCameraStoreKey& key) {
    void* raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x24, 0x24});
    if (!raw) throw std::bad_alloc();
    auto* store = new (raw) GuiCameraStore;
    store->key = key;
    store->scene = &scene.storage;
    store->camera = &camera.camera_owner().storage.node;
    store->visible_layers = 0;
    // Native AA5070 has no unwind handler for an insertion failure. Preserve
    // that contract: this low-level function does not invent ref releases.
    stores.insert_00aa5070(*store);
    return store;
}
bool remove_gui_camera_store_00aa4b30(GuiCameraStoreMap& stores, GuiCameraStore* store) noexcept {
    return stores.remove_owned_00aa4b30(store);
}
NativeGuiSceneOwner& require_gui_store_scene(const GuiCameraStore& store, NativeGuiSceneOwner& scene) {
    if (store.scene != &scene.storage)
        throw std::invalid_argument("camera store does not borrow this native scene");
    return scene;
}
NativeCameraReference& require_gui_store_camera(const GuiCameraStore& store, NativeCameraReference& camera) {
    if (store.camera != &camera.camera_owner().storage.node)
        throw std::invalid_argument("camera store does not borrow this native camera");
    return camera;
}
} // namespace bsp
