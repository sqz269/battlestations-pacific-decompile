#include "bsp/native_material_factory.hpp"
#include "bsp/native_material_lifetime.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include <stdexcept>
#include <cstdlib>

namespace bsp {
class NativeMaterialFactoryIssuer final {
public:
    explicit NativeMaterialFactoryIssuer(NativeMaterialFrame& f) noexcept:frame_(f){}
    void acquire_entered() noexcept {frame_.effect_acquisition_entered();}
    void acquire_returned(void* effect) noexcept {frame_.effect_acquisition_returned(effect);}
    void allocate_entered() noexcept {frame_.native_allocation_entered();}
    void allocate_returned(void* slot){frame_.native_allocation_returned(slot);}
    NativeMaterialStorage* initialize(void* slot,void* effect,NativeRenderActualOwners& owners){
        return NativeMaterialStorageAccess::initialize(slot,effect,owners,frame_);}
    bool may_return() const noexcept {return frame_.may_select_constructor_raw_return();}
    void return_selected(void* current,NativeMaterialSlotPool& pool){frame_.selected_constructor_raw_return(current,pool);}
    void release_entered(void* effect) noexcept {frame_.effect_release_entered(effect);}
    void release_returned() noexcept {frame_.effect_release_returned();}
    void release_effect(NativeRenderActualOwners& owners,void* actual){frame_.release_factory_effect(owners,actual);}
private:
    NativeMaterialFrame& frame_;
};
namespace {
NativeMaterialPool* canonical_material_pool;
NativeMaterialParameterPool* canonical_parameter_pool;
} // namespace

NativeMaterialFactoryAcquired::NativeMaterialFactoryAcquired() = default;
NativeMaterialFactoryAcquired::~NativeMaterialFactoryAcquired() = default;
NativeMaterialStorage* create_native_material_from_effect_cache_00535320(
    NativeString& name, NativeMaterialSlotPool& pool, NativeRenderActualOwners& owners,
    NativeMaterialEffectCacheContext& cache, NativeMaterialFactoryAcquired& a) {
    return create_native_material_from_effect_cache_00535320(&name,pool,owners,cache,a);
}

namespace {
NativeMaterialStorage* cached_body(
    const void* name, NativeMaterialSlotPool& pool, NativeRenderActualOwners& owners,
    NativeMaterialEffectCacheContext& cache, NativeMaterialFactoryAcquired& a,NativeMaterialFactoryIssuer* issuer) {
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
    if(issuer)issuer->acquire_entered();
    void* const actual_effect_return=load_native_renderer_material_effect_00b318b0(renderer,name,cache,a.cache.get());
    a.effect=actual_effect_return;
    if(issuer)issuer->acquire_returned(actual_effect_return);
    a.phase=NativeMaterialFactoryAcquired::Phase::allocation; a.native_site=0x0053534e;
    if(issuer)issuer->allocate_entered();
    void* const actual_allocation_return=allocate_native_material_slot_00b18780(pool);
    a.raw_slot=actual_allocation_return;
    if(issuer)issuer->allocate_returned(actual_allocation_return);
    if (a.raw_slot) {
        a.phase=NativeMaterialFactoryAcquired::Phase::construction; a.native_site=0x00535364;
        try { a.material=issuer?issuer->initialize(a.raw_slot,a.effect,owners):
            initialize_native_material_00b18900(a.raw_slot,a.effect,owners); }
        catch (...) {
            if(!issuer||issuer->may_return()){
                // CURRENT public HOME at the actual selected-call epoch. A
                // differing target is never substituted with captured A.
                if(issuer)issuer->return_selected(a.raw_slot,pool);
                else return_native_material_raw_slot_00b17d70(a.raw_slot,pool);
                a.raw_slot=nullptr;
            }
            throw;
        }
    }
    a.raw_slot=nullptr;
    a.phase=NativeMaterialFactoryAcquired::Phase::effect_release; a.native_site=0x00535377;
    void* effect=a.effect; a.effect=nullptr; a.cache->caller_acquired=false;
    if(issuer)issuer->release_entered(effect);
    if(issuer)issuer->release_effect(owners,effect);else release_native_render_actual_owner(owners,effect);
    if(issuer)issuer->release_returned();
    a.phase=NativeMaterialFactoryAcquired::Phase::complete; return a.material;
}

NativeMaterialStorage* callable_body(
    NativeString& name, void* const volatile& current_renderer,
    NativeMaterialSlotPool& pool, NativeRenderActualOwners& owners,NativeMaterialFactoryIssuer* issuer) {
    void* const renderer = current_renderer;
    const auto* const table = *static_cast<const std::uintptr_t* const*>(renderer);
    using Acquire = void* (__thiscall*)(void*, NativeString*);
    if(issuer)issuer->acquire_entered();
    void* const effect = reinterpret_cast<Acquire>(table[0x48 / 4])(renderer, &name);
    if(issuer)issuer->acquire_returned(effect);
    if(issuer)issuer->allocate_entered();
    void* const slot = allocate_native_material_slot_00b18780(pool);
    if(issuer)issuer->allocate_returned(slot);
    NativeMaterialStorage* material = nullptr;
    if (slot) {
        try {
            material = issuer?issuer->initialize(slot,effect,owners):initialize_native_material_00b18900(slot, effect, owners);
        } catch (...) {
            if(issuer){if(issuer->may_return())issuer->return_selected(slot,pool);}
            else return_native_material_raw_slot_00b17d70(slot, pool);
            throw;
        }
    }
    if(issuer)issuer->release_entered(effect);
    if(issuer)issuer->release_effect(owners,effect);else release_native_render_actual_owner(owners, effect);
    if(issuer)issuer->release_returned();
    return material;
}

} // anonymous shared factory kernels

NativeMaterialStorage* create_native_material_from_effect_cache_00535320(
    const void* name,NativeMaterialSlotPool& pool,NativeRenderActualOwners& owners,
    NativeMaterialEffectCacheContext& cache,NativeMaterialFactoryAcquired& a){
    return cached_body(name,pool,owners,cache,a,nullptr);
}
NativeMaterialStorage* create_native_material_for_effect_00535320(
    NativeString& name,void* const volatile& renderer,NativeMaterialSlotPool& pool,NativeRenderActualOwners& owners){
    return callable_body(name,renderer,pool,owners,nullptr);
}
NativeMaterialStorage* NativeMaterialFactoryAccess::cached(const void* name,NativeMaterialSlotPool& pool,
    NativeRenderActualOwners& owners,NativeMaterialEffectCacheContext& cache,NativeMaterialFactoryAcquired& a,
    NativeMaterialFrame& frame){NativeMaterialFactoryIssuer issuer(frame);return cached_body(name,pool,owners,cache,a,&issuer);}
NativeMaterialStorage* NativeMaterialFactoryAccess::callable(NativeString& name,void* const volatile& renderer,
    NativeMaterialSlotPool& pool,NativeRenderActualOwners& owners,NativeMaterialFrame& frame){
    NativeMaterialFactoryIssuer issuer(frame);return callable_body(name,renderer,pool,owners,&issuer);}

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
