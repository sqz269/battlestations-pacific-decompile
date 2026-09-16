#define _CRT_SECURE_NO_WARNINGS
#include "bsp/native_particle_object_resources_raw.hpp"
#include "bsp/native_particle_type_resources.hpp"
#include "bsp/native_resource_manager_lifetime.hpp"
#include "bsp/native_resource_load_cache.hpp"
#include "bsp/native_resource_container_lifetime.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <optional>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U p) noexcept { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(p)); }
void* at(void* p,U offset) noexcept { return ptr(bits(p)+offset); }
U word(const void* p,U offset=0) noexcept { U v;std::memcpy(&v,static_cast<const unsigned char*>(p)+offset,4);return v; }
void put(void* p,U offset,U v) noexcept { std::memcpy(at(p,offset),&v,4); }
}

void clear_native_object_particle_models_00af8940(void* definition,NativeResourceContainerReferences& refs) {
    while(word(definition,0x90)!=0) {
        const U count=word(definition,0x90);
        void* const cell=ptr(word(definition,0x8c)+count*4u-4u);
        void* const resource=ptr(word(cell));
        if(resource) {
            if(InterlockedDecrement(static_cast<volatile LONG*>(at(resource,4)))==0) {
                const U table=word(resource);
                const U target=word(ptr(table));
                refs.source_zero_reference(target,resource,table);
            }
            put(cell,0,0);
        }
        const U current=word(definition,0x90);
        if(current)put(definition,0x90,current-1u);
    }
}

void* load_native_resource_with_default_factory_00b80d70(void* manager,const void* name,
    NativeResourceLoadCacheContext& context,NativeResourceLoadCacheAcquired& acquired) {
    void* const factory=ptr(word(manager,4));
    return load_and_cache_native_resource_00b80720(manager,name,factory,context,acquired);
}

struct NativeParticleObjectResourcesRawAcquired::Impl {
    // Offsets are native ESP after all three saved registers; local10..847.
    alignas(8) unsigned char locals[0x838];
    std::optional<NativeVfsNameResolutionAcquired> resolution;
    std::optional<NativeResourceLoadCacheAcquired> cache;
    NativeParticleObjectResourcesRawPhase phase=NativeParticleObjectResourcesRawPhase::fresh;
    U site=0,failure=0;std::int32_t state=-1,failed_state=-1;
    Impl() {} // Native unused local bytes remain uninitialized.
    void* local(U offset) noexcept { return locals+offset-0x10; }
    void ret(void* data,U length,NativeStringRawPoolContext& strings,U getter,U release) {
        if(!data)return;
        const U bytes=length+1u;site=getter;
        auto* const pool=native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8,strings.actual_manager_publication_01090aa0);
        site=release;
        return_native_string_pool_00bd1510(pool,data,bytes,strings.actual_small_returns_disabled_01090aa4);
    }
    void ret_header(U offset,std::int32_t next,NativeStringRawPoolContext& strings,U getter,U release) {
        void* const data=ptr(word(local(offset),4));state=next;
        if(data)ret(data,word(local(offset)),strings,getter,release);
    }
    void copy(U destination,const void* source,NativeStringRawPoolContext& strings,U resize,U copy_site) {
        void* const output=local(destination);
        if(output==source)return;
        const U length=word(source);site=resize;
        resize_native_string_header_0041dd40(output,strings,length,true);
        if(word(source)!=0) {
            const U count=word(output);const void* const input=ptr(word(source,4));
            void* const data=ptr(word(output,4));site=copy_site;std::memmove(data,input,count);
        }
    }
    bool resolve(U offset,U call_site,NativeParticleObjectResourcesRawContext& context) {
        // A previous successful child no longer references the reused header.
        // A failed child cannot reach this point or be destroyed for reuse.
        resolution.emplace();site=call_site;
        void* const vfs=context.cache.actual_vfs_0109ceec;
        return resolve_native_vfs_existing_name_00bdf4c0(vfs,local(offset),context.cache.names,*resolution);
    }
    void* fetch(U offset,bool numbered,NativeParticleObjectResourcesRawContext& context) {
        cache.emplace();
        const bool explicit_factory=context.factory_alias_00f8d31c!=nullptr;
        site=numbered?(explicit_factory?0x00af9994u:0x00af99aeu):(explicit_factory?0x00af9ab8u:0x00af9ad2u);
        void* const manager=get_native_resource_manager_004c1400(context.manager);
        if(explicit_factory) {
            void* const factory=context.factory_alias_00f8d31c;
            site=numbered?0x00af99a7u:0x00af9acbu;
            return load_and_cache_native_resource_00b80720(manager,local(offset),factory,context.cache,*cache);
        }
        site=numbered?0x00af99bau:0x00af9adeu;
        return load_native_resource_with_default_factory_00b80d70(manager,local(offset),context.cache,*cache);
    }
    void append(void* definition,void* resource,U reserve_site) {
        void* const descriptor=at(definition,0x8c);const U capacity=word(descriptor,8);
        if(word(descriptor,4)==capacity) {
            auto wanted=static_cast<std::int32_t>(capacity*2u);if(wanted<=1)wanted=1;
            site=reserve_site;reserve_native_object_particle_models_00af8350(descriptor,wanted);
        }
        void* const cell=ptr(word(descriptor)+word(descriptor,4)*4u);
        if(cell)put(cell,0,bits(resource));
        put(descriptor,4,word(descriptor,4)+1u);
    }
    void cleanup(NativeStringRawPoolContext& strings) noexcept {
        static constexpr std::int32_t previous[]={-1,0,1,1,1,4,5,6,5,4,1};
        static constexpr U offsets[]={0x10,0x30,0x28,0x28,0x28,0x20,0x40,0x18,0x18,0x18,0x20};
        try {
            while(state>=0) { const auto current=state;state=previous[current];
                destroy_native_string_header_0041dd20(local(offsets[current]),strings); }
        } catch(...) { std::terminate(); }
    }
};
NativeParticleObjectResourcesRawAcquired::NativeParticleObjectResourcesRawAcquired():impl_(std::make_unique<Impl>()) {}
NativeParticleObjectResourcesRawAcquired::~NativeParticleObjectResourcesRawAcquired()=default;
NativeParticleObjectResourcesRawPhase NativeParticleObjectResourcesRawAcquired::phase() const noexcept {return impl_->phase;}
U NativeParticleObjectResourcesRawAcquired::failure_site() const noexcept {return impl_->failure;}
std::int32_t NativeParticleObjectResourcesRawAcquired::native_state_at_failure() const noexcept {return impl_->failed_state;}
const NativeVfsNameResolutionAcquired* NativeParticleObjectResourcesRawAcquired::resolution_invocation() const noexcept {return impl_->resolution?&*impl_->resolution:nullptr;}
const NativeResourceLoadCacheAcquired* NativeParticleObjectResourcesRawAcquired::cache_invocation() const noexcept {return impl_->cache?&*impl_->cache:nullptr;}

void load_native_object_particle_models_00af9660(void* definition,const char* filename,
    NativeParticleObjectResourcesRawContext& context,NativeParticleObjectResourcesRawAcquired& acquired) {
    auto& f=*acquired.impl_;
    if(f.phase!=NativeParticleObjectResourcesRawPhase::fresh)throw std::logic_error("Object model invocation cannot replay");
    f.phase=NativeParticleObjectResourcesRawPhase::running;auto& strings=context.manager.strings;
    put(f.local(0x38),0,bits(definition));
    try {
        f.site=0x00af9688;clear_native_object_particle_models_00af8940(definition,context.references);
        f.site=0x00af9695;construct_native_string_header_0041e870(f.local(0x10),strings,filename);
        f.state=0;(void)f.resolve(0x10,0x00af96ae,context);
        put(f.local(0x18),0,0);put(f.local(0x18),4,0);f.site=0x00af96c3;
        resize_native_string_header_0041dd40(f.local(0x18),strings,1,true);
        void* const delimiter=ptr(word(f.local(0x18),4));const U delimiter_length=word(f.local(0x18));
        if(delimiter) { f.site=0x00af96de;std::memmove(delimiter,".",delimiter_length+1u); }
        f.site=0x00af96f4;const auto dot=reverse_find_native_string_header_00467cf0(f.local(0x10),f.local(0x18),0x7fffffffu);
        f.ret(delimiter,delimiter_length,strings,0x00af9706,0x00af970d);
        put(f.local(0x30),0,0);put(f.local(0x30),4,0);f.state=1;
        if(dot!=-1) {
            f.site=0x00af973c;const void* const extension=construct_native_string_substring_00469840(f.local(0x10),f.local(0x28),static_cast<U>(dot),0x7fffffffu,strings);
            f.state=2;f.copy(0x30,extension,strings,0x00af975a,0x00af9772);
            f.ret_header(0x28,1,strings,0x00af9795,0x00af979c);
            f.site=0x00af97ad;const void* const stem=construct_native_string_substring_00469840(f.local(0x10),f.local(0x28),0,static_cast<U>(dot),strings);
            f.state=3;f.copy(0x10,stem,strings,0x00af97c9,0x00af97e1);
            f.ret_header(0x28,1,strings,0x00af9804,0x00af980b);
        }
        auto index=static_cast<std::int32_t>(word(f.local(0x10))-1u);U digits=0;
        const char* scan=nullptr;
        if(index>0) {
            scan=static_cast<const char*>(ptr(word(f.local(0x10),4)));
            do { const auto c=static_cast<signed char>(scan[index]);if(c<'0'||c>'9')break;
                --index;++digits; } while(index>0);
        }
        if(digits) {
            ++index;if(!scan)scan=context.empty_stem_00f8d320;f.site=0x00af9853;
            const U number=static_cast<U>(std::atol(scan+index));put(f.local(0x3c),0,number);
            f.site=0x00af986a;std::sprintf(static_cast<char*>(f.local(0x448)),"%%0%dd",static_cast<std::int32_t>(digits));
            f.site=0x00af987f;construct_native_string_substring_00469840(f.local(0x10),f.local(0x28),0,static_cast<U>(index),strings);f.state=4;
            for(;;) {
                const U captured=word(f.local(0x3c));auto* const buffer=static_cast<char*>(f.local(0x48));buffer[0]=0;
                f.site=0x00af98a7;std::sprintf(buffer,static_cast<const char*>(f.local(0x448)),static_cast<std::int32_t>(captured));
                put(f.local(0x3c),0,captured+1u);put(f.local(0x20),0,0);put(f.local(0x20),4,0);
                const U length=static_cast<U>(std::strlen(buffer));f.site=0x00af98d7;
                resize_native_string_header_0041dd40(f.local(0x20),strings,length,true);
                void* const suffix=ptr(word(f.local(0x20),4));const U suffix_length=word(f.local(0x20));
                if(suffix) { f.site=0x00af98f2;std::memmove(suffix,buffer,suffix_length+1u); }
                f.state=5;f.site=0x00af9910;
                const void* const head=concatenate_native_string_headers_004261a0(f.local(0x28),f.local(0x40),f.local(0x20),strings);
                f.state=6;f.site=0x00af9929;concatenate_native_string_headers_004261a0(head,f.local(0x18),f.local(0x30),strings);
                f.ret_header(0x40,8,strings,0x00af9949,0x00af9950);
                f.state=9;f.ret(suffix,suffix_length,strings,0x00af9968,0x00af996f);
                if(!f.resolve(0x18,0x00af997f,context)) { f.ret_header(0x18,4,strings,0x00af9a44,0x00af9a4b);break; }
                void* const resource=f.fetch(0x18,true,context);
                f.append(ptr(word(f.local(0x38))),resource,0x00af99e2);
                f.ret_header(0x18,4,strings,0x00af9a18,0x00af9a1f);
            }
            f.ret_header(0x28,1,strings,0x00af9a6f,0x00af9a76);
        } else {
            f.site=0x00af9a8e;concatenate_native_string_headers_004261a0(f.local(0x10),f.local(0x20),f.local(0x30),strings);
            f.state=10;
            if(f.resolve(0x20,0x00af9aa6,context)) {
                void* const resource=f.fetch(0x20,false,context);
                f.append(ptr(word(f.local(0x38))),resource,0x00af9b06);
            }
            f.ret_header(0x20,1,strings,0x00af9b38,0x00af9b3f);
        }
        f.ret_header(0x30,0,strings,0x00af9b61,0x00af9b68);
        f.ret_header(0x10,-1,strings,0x00af9b8b,0x00af9b92);
        f.phase=NativeParticleObjectResourcesRawPhase::complete;
    } catch(...) {
        f.failure=f.site;f.failed_state=f.state;f.phase=NativeParticleObjectResourcesRawPhase::failed;
        f.cleanup(strings);throw;
    }
}
} // namespace bsp
