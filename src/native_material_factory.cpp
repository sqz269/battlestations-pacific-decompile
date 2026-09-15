#include "bsp/native_material_factory.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include <stdexcept>
#include <cstdlib>

namespace bsp {
namespace {
NativeMaterialPool* canonical_material_pool;
NativeMaterialParameterPool* canonical_parameter_pool;
} // namespace

NativeMaterialFactoryAcquired::NativeMaterialFactoryAcquired() = default;
NativeMaterialFactoryAcquired::~NativeMaterialFactoryAcquired() = default;
NativeMaterialStorage* create_native_material_from_effect_cache_00535320(
    NativeString& name, NativeMaterialSlotPool& pool, NativeRenderActualOwners& owners,
    NativeMaterialEffectCacheContext& cache, NativeMaterialFactoryAcquired& a) {
    if (a.phase != NativeMaterialFactoryAcquired::Phase::empty || a.cache || a.effect || a.material ||
        a.raw_slot || a.companion || a.owner_record || a.registered ||
        &cache.effects.owners.actual_owners() != &owners)
        throw std::invalid_argument("native material factory requires a fresh frame and shared effect owners");
    a.cache=std::make_unique<NativeMaterialEffectCacheAcquired>();
    a.phase=NativeMaterialFactoryAcquired::Phase::effect; a.native_site=0x00535345;
    void* renderer=cache.effects.construction.current_renderer_00f8d394;
    const auto* profile=cache.effects.renderer_profile_00d5f0a8;
    if (!renderer || *static_cast<const volatile std::uint32_t*>(renderer)!=0x00d5f0a8u ||
        !profile || profile[0x48/4]!=0x00b318b0u)
        throw std::logic_error("native material factory requires current renderer48 B318B0");
    a.effect=load_native_renderer_material_effect_00b318b0(renderer,&name,cache,a.cache.get());
    a.phase=NativeMaterialFactoryAcquired::Phase::allocation; a.native_site=0x0053534e;
    a.raw_slot=allocate_native_material_slot_00b18780(pool);
    if (a.raw_slot) {
        a.phase=NativeMaterialFactoryAcquired::Phase::construction; a.native_site=0x00535364;
        try { a.material=initialize_native_material_00b18900(a.raw_slot,a.effect,owners); }
        catch (...) { return_native_material_raw_slot_00b17d70(a.raw_slot,pool); a.raw_slot=nullptr; throw; }
    }
    a.raw_slot=nullptr;
    a.phase=NativeMaterialFactoryAcquired::Phase::effect_release; a.native_site=0x00535377;
    void* effect=a.effect; a.effect=nullptr; a.cache->caller_acquired=false;
    release_native_render_actual_owner(owners,effect);
    a.phase=NativeMaterialFactoryAcquired::Phase::complete; return a.material;
}

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
