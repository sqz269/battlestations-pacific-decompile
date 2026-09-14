#include "bsp/native_material_factory.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstdlib>
#include <stdexcept>

namespace bsp {
namespace {
NativeMaterialPool* canonical_material_pool;
NativeMaterialParameterPool* canonical_parameter_pool;
NativeMaterialStorage* complete_material(void* effect, NativeMaterialSlotPool& pool,
    NativeRenderActualOwners& owners, NativeMaterialStorage** completed) {
    void* const slot = allocate_native_material_slot_00b18780(pool);
    NativeMaterialStorage* material = nullptr;
    if (slot) {
        try {
            material = initialize_native_material_00b18900(slot, effect, owners);
        } catch (...) {
            return_native_material_raw_slot_00b17d70(slot, pool);
            throw;
        }
    }
    if (completed) *completed = material;
    release_native_render_actual_owner(owners, effect);
    return material;
}
} // namespace

NativeMaterialStorage* create_native_material_for_effect_00535320(
    NativeString& name, void* const volatile& current_renderer,
    NativeMaterialSlotPool& pool, NativeRenderActualOwners& owners) {
    void* const renderer = current_renderer;
    const auto* const table = *static_cast<const std::uintptr_t* const*>(renderer);
    using Acquire = void* (__thiscall*)(void*, NativeString*);
    void* const effect = reinterpret_cast<Acquire>(table[0x48 / 4])(renderer, &name);
    return complete_material(effect, pool, owners, nullptr);
}

NativeMaterialStorage* create_native_material_for_effect_00535320(
    NativeString& name, void* const volatile& current_renderer,
    NativeMaterialSlotPool& pool, NativeRenderActualOwners& owners,
    NativeMaterialFactoryRawContext& raw, NativeMaterialStorage** completed) {
    if ((completed && *completed) ||
        &raw.effects.effects.owners.actual_owners() != &owners ||
        &raw.effects.effects.owners.string_storage() != &raw.effects.strings ||
        &raw.effects.effects.construction.strings != &raw.effects.strings ||
        &raw.effects.effects.construction.current_renderer_00f8d394 != &current_renderer)
        throw std::invalid_argument("material factory requires an empty output and the same actual renderer, strings and owners");
    void* const renderer = current_renderer;
    if (!renderer || *static_cast<const volatile std::uint32_t*>(renderer) != 0x00d5f0a8 ||
        !raw.renderer_profile_00d5f0a8 || raw.renderer_profile_00d5f0a8[0x48 / 4] != 0x00b318b0)
        throw std::invalid_argument("material factory requires current D5F0A8 slot48 B318B0");
    void* const effect = load_native_renderer_material_effect_00b318b0(
        renderer, &name, raw.effects, &raw.acquired_effect);
    if (!effect)
        throw std::invalid_argument("raw material creation requires a nonnull effect; native null-effect fault is outside the source domain");
    return complete_material(effect, pool, owners, completed);
}

void return_native_material_raw_slot_00b17d70(void* slot, NativeMaterialSlotPool& pool) {
    pool.return_slot_00b17a80(slot);
}

void bind_static_native_material_pool_00f8d3ac(NativeMaterialPool& pool) noexcept {
    canonical_material_pool = &pool;
}
void bind_static_native_material_parameter_pool_00f8d3e4(NativeMaterialParameterPool& pool) noexcept {
    canonical_parameter_pool = &pool;
}
int initialize_static_native_material_pool_00cd78d0() {
    canonical_material_pool->initialize_00b17fa0();
    return std::atexit(&destroy_static_native_material_pool_00ce0bf0);
}
int initialize_static_native_material_parameter_pool_00cd78f0() {
    canonical_parameter_pool->initialize_00b18340();
    return std::atexit(&destroy_static_native_material_parameter_pool_00ce0c00);
}
void destroy_static_native_material_pool_00ce0bf0() noexcept {
    canonical_material_pool->destroy_00b180d0();
}
void destroy_static_native_material_parameter_pool_00ce0c00() noexcept {
    canonical_parameter_pool->destroy_00b18470();
}
} // namespace bsp
