#include "bsp/native_material_effect_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
constexpr std::uint32_t reference_table=0x00ceb130,base_table=0x00d5e534,effect_table=0x00d61a00;
struct BaseCleanup {
    NativeMaterialEffectBaseStorage& storage;
    ~BaseCleanup(){storage.vtable_00=reference_table;}
};
struct NameCleanup {
    NativeString& header;NativeStringStorage& strings;
    ~NameCleanup(){destroy_native_string_header_0041dd20(&header,strings);}
};
struct NamesCleanup {
    NativeString* headers;NativeStringStorage& strings;
    ~NamesCleanup(){destroy_native_material_effect_names_00b18d50(headers,strings);}
};
struct EffectBaseCleanup {
    NativeMaterialEffectBaseStorage& storage;NativeMaterialEffectDestructionAccess& access;
    ~EffectBaseCleanup() noexcept(false){destroy_native_material_effect_base_00b18eb0(storage,access);}
};
void require_storage(void* slot) {
    if(!slot || reinterpret_cast<std::uintptr_t>(slot)%alignof(NativeMaterialEffectBaseStorage))
        throw std::invalid_argument("native effect requires aligned fresh storage");
}
void release_then_clear(void*& slot,NativeRenderActualOwners& owners) {
    void* const old=slot;
    if(!old)return;
    release_native_render_actual_owner(owners,old);
    slot=nullptr;
}
std::size_t texture_end(const NativeMaterialEffectBaseStorage& storage) {
    const auto count=storage.texture_count_38;
    if(count<0 || count>11)throw std::logic_error("native effect texture extent outside0..11");
    return static_cast<std::size_t>(count);
}
std::size_t retained_end(const NativeMaterialEffectBaseStorage& storage) {
    const auto count=storage.retained_count_a8;
    if(count<0 || count>3)throw std::logic_error("native effect retained extent outside0..3");
    return static_cast<std::size_t>(count);
}
void initialize_base(NativeMaterialEffectBaseStorage& storage,NativeMaterialEffectConstructionAccess& access,
    NativeTextureCacheContext* cache = nullptr, NativeTextureCacheAcquired* acquired = nullptr,
    const volatile std::uint32_t* renderer_profile = nullptr) {
    storage.vtable_00=reference_table;
    storage.references_04.store(1,std::memory_order_relaxed);
    storage.vtable_00=base_table;
    storage.texture_count_38=0;
    storage.textures_0c.fill(nullptr);
    // NativeString construction is nonthrowing; the native array helper also
    // creates eleven empty headers, then repeats their zero stores via memset.
    storage.name_count_94=0;
    std::memset(static_cast<void*>(storage.names_3c.data()),0,sizeof(storage.names_3c));
    storage.retained_count_a8=0;
    ::new (&storage.name_b8) NativeString;
    try {
        NativeString temporary;
        resize_native_string_header_0041dd40(&temporary,access.strings,9,true);
        if(temporary.data())std::memcpy(temporary.data(),"error.tga",temporary.length()+1u);
        {
            // Native state3 begins only AFTER resize/copy, before virtual+64.
            const NameCleanup cleanup{temporary,access.strings};
            void* const renderer=access.current_renderer_00f8d394;
            if (cache) {
                if (!renderer || *static_cast<const volatile std::uint32_t*>(renderer) != 0x00d5f0a8u ||
                    !renderer_profile || renderer_profile[0x64/4] != 0x00b319b0u)
                    throw std::logic_error("native effect construction requires current renderer64 B319B0");
                storage.fallback_98=load_native_renderer_texture_00b319b0(renderer,&temporary,0,*cache,acquired);
                acquired->caller_acquired=false; // Transferred to +98; no reference operation.
            } else {
                const auto* const table=*static_cast<const std::uintptr_t* const*>(renderer);
                using Acquire=void* (__thiscall*)(void*,NativeString*,std::uint32_t);
                storage.fallback_98=reinterpret_cast<Acquire>(table[0x64/4])(renderer,&temporary,0);
            }
        }
        const auto serial=access.next_serial_00f8d3a8;
        storage.serial_c0=serial;
        storage.byte_08=0;
        storage.dirty_b4=0;
        access.next_serial_00f8d3a8=serial+1u;
    } catch(...) {
        // Original states2/1/0: member name, reverse header array, base only.
        // No fallback/texture/derived-owner cleanup is armed in this constructor.
        destroy_native_string_header_0041dd20(&storage.name_b8,access.strings);
        destroy_native_material_effect_names_00b18d50(storage.names_3c.data(),access.strings);
        storage.vtable_00=reference_table;
        throw;
    }
}
void require_texture_construction_domain(NativeMaterialEffectConstructionAccess& access,
    NativeTextureCacheContext& cache, NativeTextureCacheAcquired& acquired) {
    if (&access.strings != &cache.strings ||
        static_cast<const volatile void*>(&access.current_renderer_00f8d394) !=
            static_cast<const volatile void*>(&cache.textures.current_renderer_00f8d394) ||
        cache.textures.cache != &cache ||
        acquired.phase != NativeTextureCacheAcquired::Phase::not_started || acquired.wrapper_started)
        throw std::invalid_argument("native effect texture construction requires one fresh frame and shared actual domains");
}
NativeMaterialEffectStorage* finish_derived(NativeMaterialEffectStorage* storage) {
    storage->descriptor_c4=nullptr;
    storage->retained_138=nullptr;
    storage->byte_13c=0;
    storage->base.vtable_00=effect_table;
    storage->words_140.fill(0);
    return storage;
}
} // namespace

NativeMaterialEffectBaseStorage* initialize_native_material_effect_base_00b18d60(
    void* slot,NativeMaterialEffectConstructionAccess& access) {
    require_storage(slot);
    auto* storage=::new(slot) NativeMaterialEffectBaseStorage;
    initialize_base(*storage,access);return storage;
}
NativeMaterialEffectStorage* initialize_native_material_effect_00b407a0(
    void* slot,NativeMaterialEffectConstructionAccess& access) {
    require_storage(slot);
    auto* storage=::new(slot) NativeMaterialEffectStorage;
    initialize_base(storage->base,access);
    return finish_derived(storage);
}
NativeMaterialEffectBaseStorage* initialize_native_material_effect_base_with_texture_cache_00b18d60(
    void* slot, NativeMaterialEffectConstructionAccess& access, NativeTextureCacheContext& cache,
    NativeTextureCacheAcquired& acquired, const volatile std::uint32_t* profile) {
    require_storage(slot); require_texture_construction_domain(access,cache,acquired);
    auto* storage=::new(slot) NativeMaterialEffectBaseStorage;
    initialize_base(*storage,access,&cache,&acquired,profile); return storage;
}
NativeMaterialEffectStorage* initialize_native_material_effect_with_texture_cache_00b407a0(
    void* slot, NativeMaterialEffectConstructionAccess& access, NativeTextureCacheContext& cache,
    NativeTextureCacheAcquired& acquired, const volatile std::uint32_t* profile) {
    require_storage(slot); require_texture_construction_domain(access,cache,acquired);
    auto* storage=::new(slot) NativeMaterialEffectStorage;
    initialize_base(storage->base,access,&cache,&acquired,profile); return finish_derived(storage);
}
void destroy_native_material_effect_names_00b18d50(NativeString* names,NativeStringStorage& strings) noexcept {
    for(std::size_t i=11;i!=0;--i)destroy_native_string_header_0041dd20(names+i-1,strings);
}
void release_native_material_effect_base_owners_00b187a0(
    NativeMaterialEffectBaseStorage& storage,NativeRenderActualOwners& owners) {
    for(std::size_t i=0;i!=texture_end(storage);++i) {
        if(i>=11)throw std::logic_error("native effect texture end moved behind release cursor");
        release_then_clear(storage.textures_0c[i],owners);
    }
    storage.texture_count_38=0;
    for(std::size_t i=0;i!=retained_end(storage);++i) {
        if(i>=3)throw std::logic_error("native effect retained end moved behind release cursor");
        release_then_clear(storage.retained_9c[i],owners);
    }
    storage.retained_count_a8=0;
}
void release_native_material_effect_owners_00b41b10(
    NativeMaterialEffectStorage& storage,NativeRenderActualOwners& owners) {
    void* const descriptor=storage.descriptor_c4;
    if(descriptor) {
        const auto* const table=*static_cast<const std::uintptr_t* const*>(descriptor);
        using Delete=void (__thiscall*)(void*,std::uint32_t);
        reinterpret_cast<Delete>(table[0])(descriptor,1);
        storage.descriptor_c4=nullptr;
    }
    for(void*& slot:storage.passes_c8)release_then_clear(slot,owners);
    for(void*& slot:storage.secondary_100)release_then_clear(slot,owners);
    release_then_clear(storage.retained_138,owners);
}
void destroy_native_material_effect_base_00b18eb0(
    NativeMaterialEffectBaseStorage& storage,NativeMaterialEffectDestructionAccess& access) {
    storage.vtable_00=base_table;
    const BaseCleanup base{storage};
    const NamesCleanup names{storage.names_3c.data(),access.strings};
    const NameCleanup name{storage.name_b8,access.strings};
    release_then_clear(storage.fallback_98,access.retained_owners);
    release_native_material_effect_base_owners_00b187a0(storage,access.retained_owners);
}
void destroy_native_material_effect_00b41f80(
    NativeMaterialEffectStorage& storage,NativeMaterialEffectDestructionAccess& access) {
    storage.base.vtable_00=effect_table;
    const EffectBaseCleanup base{storage.base,access};
    release_native_material_effect_owners_00b41b10(storage,access.retained_owners);
}
NativeMaterialEffectBaseStorage* delete_native_material_effect_base_00b192d0(
    NativeMaterialEffectBaseStorage* storage,NativeMaterialEffectDestructionAccess& access,std::uint32_t flags) {
    destroy_native_material_effect_base_00b18eb0(*storage,access);
    if(flags&1)singleton_lifetime_free(storage);
    return storage;
}
NativeMaterialEffectStorage* delete_native_material_effect_00b422d0(
    NativeMaterialEffectStorage* storage,NativeMaterialEffectDestructionAccess& access,std::uint32_t flags) {
    destroy_native_material_effect_00b41f80(*storage,access);
    if(flags&1)singleton_lifetime_free(storage);
    return storage;
}

NativeMaterialEffectReference::NativeMaterialEffectReference(NativeMaterialEffectBaseStorage& storage,
    NativeMaterialEffectDestructionAccess& access,const volatile std::uint32_t* profile,
    NativeMaterialEffectCompanionDisposal disposal)
    :RenderCommandReference(storage.references_04),storage_(storage),access_(access),profile_(profile),
     disposal_(disposal),bound_table_(storage.vtable_00) {
    if(!disposal.retire || storage.references_04.load(std::memory_order_relaxed)<=0)
        throw std::invalid_argument("native effect reference requires live storage and explicit retirement");
    require_current_profile();
}
NativeMaterialEffectReference::~NativeMaterialEffectReference() {
    if(phase_!=Phase::retired)std::terminate();
}
void NativeMaterialEffectReference::require_current_profile() const noexcept {
    if(storage_.vtable_00!=bound_table_ || !profile_ || profile_[0]!=0x00bd30e0)std::terminate();
    if(bound_table_==base_table) {if(profile_[1]!=0x00b192d0)std::terminate();}
    else if(bound_table_==effect_table) {if(profile_[1]!=0x00b422d0)std::terminate();}
    else std::terminate();
}
void NativeMaterialEffectReference::release_zero_references() noexcept {
    if(phase_!=Phase::bound)std::terminate();
    require_current_profile();phase_=Phase::destroying;
    const auto disposal=disposal_;
    try {
        if(bound_table_==base_table)delete_native_material_effect_base_00b192d0(&storage_,access_,1);
        else delete_native_material_effect_00b422d0(reinterpret_cast<NativeMaterialEffectStorage*>(&storage_),access_,1);
    } catch(...) {std::terminate();}
    phase_=Phase::retired;
    disposal.retire(disposal.context,*this);
}
} // namespace bsp
