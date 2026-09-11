#include "bsp/native_material_factory.hpp"
#include <cstdlib>

namespace bsp {
namespace {
NativeMaterialPool* canonical_material_pool;
NativeMaterialParameterPool* canonical_parameter_pool;
} // namespace

NativeMaterialStorage* create_native_material_for_effect_00535320(
    NativeString& name, void* const volatile& current_renderer,
    NativeMaterialSlotPool& pool, NativeRenderActualOwners& owners) {
    void* const renderer = current_renderer;
    const auto* const table = *static_cast<const std::uintptr_t* const*>(renderer);
    using Acquire = void* (__thiscall*)(void*, NativeString*);
    void* const effect = reinterpret_cast<Acquire>(table[0x48 / 4])(renderer, &name);
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
    release_native_render_actual_owner(owners, effect);
    return material;
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
