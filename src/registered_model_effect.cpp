#include "bsp/registered_model_effect.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(RegisteredModelEffectStorage) == 0x20);
static_assert(offsetof(RegisteredModelEffectStorage, prefix) == 0);
static_assert(offsetof(RegisteredModelEffectStorage, model_1c) == 0x1c);
template<class T> T field(const void* owner, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(owner) + offset, sizeof(value));
    return value;
}
void base_unwind(RegisteredModelEffectStorage& effect) noexcept {
    effect.prefix.table_00 = 0x00d0c88c;
    effect.prefix.secondary_table_08 = 0x00d0c888;
    effect.prefix.table_00 = 0x00ceb130; //858150 -> BD30F0.
}
NativeLiveEffectManagerStorage& manager(LiveEffectEventRegistryBindings& registry) {
    return *live_effect_manager_singleton_004d1100(
        registry.actual_manager_00f8765c, registry.domain);
}
CameraTransform& model_transform(RegisteredModelEffectRuntime& runtime,
    NativeNodeStorage* actual_model) {
    return runtime.parenting.nodes.scenes.resolve_key(
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(actual_model))).transform;
}
}

RegisteredModelEffectStorage& construct_registered_model_effect_008742a0(
    RegisteredModelEffectStorage& effect, void* definition,
    PointEffectInstanceStorage& point, RegisteredModelEffectRuntime& runtime) {
    auto& prefix = effect.prefix;
    prefix.table_00 = 0x00ceb130;
    prefix.references_04.store(1, std::memory_order_relaxed);
    prefix.secondary_table_08 = 0x00d0c8c0;
    prefix.active_0c = 1;
    prefix.borrowed_10 = &point;
    prefix.borrowed_14 = definition;
    prefix.table_00 = 0x00d0de18;
    prefix.secondary_table_08 = 0x00d0de14;
    prefix.type_18 = 1;
    try {
        const auto count = field<std::uint32_t>(definition, 0x2c);
        const auto sample = runtime.random.next_00bd2fc0(RandomStream::primary);
        if (!count) throw std::invalid_argument("Native Particle variant count must be nonzero");
        auto* const variants = field<void**>(definition, 0x28); //Reload AFTER random.
        void* const variant = variants[sample % count];
        void* const raw = runtime.callees.allocate_model_slot_00af6b60();
        NativeNodeStorage* model = nullptr;
        if (raw) {
            try { model = runtime.callees.construct_model_00af74a0(raw, variant); }
            catch (...) {
                runtime.callees.return_model_slot_00af62f0(raw); //State1 first.
                throw;
            }
        }
        effect.model_1c = model; //State0: no successful-model destructor in unwind.
        if (!model) throw std::invalid_argument("Native Particle model must be nonnull");
        if (model->vtable_00 != 0x00d5da50 || !runtime.actual_model_table_00d5da50 ||
            runtime.actual_model_table_00d5da50[0x38 / 4] != 0x00b6db10)
            throw std::invalid_argument("Unbound current particle-model virtual38");
        const CameraMatrix identity{1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
                                    0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
        set_transform_local_matrix_00b6db10(model_transform(runtime, model), identity);
        auto* const parent = point.node_110;
        set_native_node_parent_00b6e680(runtime.parenting,
            model_transform(runtime, effect.model_1c), parent ? &parent->transform : nullptr);
        CameraMatrix cached;
        std::memcpy(cached.data(), point.cached_world_90.data(), sizeof(cached));
        auto& custom_world = *reinterpret_cast<CameraMatrix*>(
            reinterpret_cast<std::byte*>(effect.model_1c) + 0x298);
        copy_camera_matrix_004134f0(custom_world, cached);
        auto& registry = runtime.registry;
        register_live_effect_event_00866a10(manager(registry), &effect,
            registry.actual_lock_00f87654, registry.lock_lifetime);
    } catch (...) {
        base_unwind(effect); //State0, even after completed model/parenting work.
        throw;
    }
    return effect;
}

RegisteredModelEffectStorage* create_registered_model_effect_0086b1c0(
    void* definition, PointEffectInstanceStorage& point, RegisteredModelEffectRuntime& runtime) {
    auto* const gate = static_cast<const volatile std::byte*>(runtime.callees.call_0051f6b0());
    if (gate[4] != std::byte{0}) return nullptr;
    void* const raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x20, 0x20});
    if (!raw) return nullptr;
    auto* const effect = ::new (raw) RegisteredModelEffectStorage;
    try { construct_registered_model_effect_008742a0(*effect, definition, point, runtime); }
    catch (...) {
        effect->~RegisteredModelEffectStorage();
        singleton_lifetime_free(raw);
        throw;
    }
    return effect;
}

void destroy_registered_model_effect_00874430(RegisteredModelEffectStorage& effect,
    LiveEffectEventRegistryBindings& registry) {
    effect.prefix.table_00 = 0x00d0de18;
    effect.prefix.secondary_table_08 = 0x00d0de14;
    try {
        unregister_live_effect_event_00866b00(manager(registry), &effect,
            registry.actual_lock_00f87654, registry.lock_lifetime);
    } catch (...) {
        base_unwind(effect);
        throw;
    }
    base_unwind(effect);
}
RegisteredModelEffectStorage* delete_registered_model_effect_008745f0(
    RegisteredModelEffectStorage& effect, std::uint32_t flags,
    LiveEffectEventRegistryBindings& registry) {
    destroy_registered_model_effect_00874430(effect, registry);
    if (flags & 1u) {
        effect.~RegisteredModelEffectStorage();
        singleton_lifetime_free(&effect);
    }
    return &effect;
}
} // namespace bsp
