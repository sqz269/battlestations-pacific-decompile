#include "bsp/native_sampler_loader_context.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Op=NativeSamplerLoaderOperation;
void begin(Op& a,std::uint32_t fn,void* volatile& manager,void* volatile& owner) {
    if(a.phase!=Op::Phase::fresh)throw std::logic_error("sampler loader operation is one-shot");
    a.phase=Op::Phase::running;a.function=fn;a.manager_publication=&manager;a.owner_publication=&owner;
}
volatile std::uint32_t& depth(void* section) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(static_cast<char*>(section)+0x18);
}
}
NativeSamplerLoaderOperation::~NativeSamplerLoaderOperation(){
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void NativeSamplerLoaderOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::failed)phase=Phase::diagnostic_retired;
}
NativeSamplerLoaderSingletonStorage* get_native_sampler_loader_singleton_004de4b0(
    void* volatile& manager,void* volatile& publication,Op& a) {
    begin(a,0x004de4b0,manager,publication);
    try {
        a.result=publication;
        if(a.result){a.phase=Op::Phase::complete;return static_cast<NativeSamplerLoaderSingletonStorage*>(a.result);}
        a.native_site=0x004de4d9;a.first_manager=get_native_singleton_manager_00415350(manager);
        a.guard.section_04=*reinterpret_cast<void* volatile*>(static_cast<char*>(a.first_manager)+0x10);
        a.guard.profile_00=0x00ce37fc;
        if(a.guard.section_04) {
            a.native_site=0x004de4f2;
            EnterCriticalSection(static_cast<CRITICAL_SECTION*>(a.guard.section_04));
            depth(a.guard.section_04)=depth(a.guard.section_04)+1u;
        }
        a.guard_armed=true;
        try {
            if(!publication) {
                a.native_site=0x004de50a;
                a.allocation=singleton_lifetime_allocate({SingletonAllocationKind::object,0x1c,0x1c});
                if(a.allocation) {
                    auto* owner=::new(a.allocation) NativeSamplerLoaderSingletonStorage;
                    owner->cache_vtable_04=0x00ce7d08;
                    owner->records_08.data_00=nullptr;
                    owner->records_08.count_04=0;
                    owner->records_08.capacity_08=0;
                    owner->word_14=0;
                    owner->vtable_00=0x00ce7d38;
                    owner->cache_vtable_04=0x00ce7d24;
                }
                publication=a.allocation;a.published=true;
                a.native_site=0x004de53f;a.current_manager=get_native_singleton_manager_00415350(manager);
                a.registration_argument=publication;a.registration_started=true;a.native_site=0x004de54d;
                register_native_singleton_object_00bd0c30(a.current_manager,nullptr,a.registration_argument);
                a.registration_returned=true;
            }
            if(a.guard.section_04) {
                depth(a.guard.section_04)=depth(a.guard.section_04)-1u;
                a.native_site=0x004de55b;
                LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(a.guard.section_04));
            }
            a.guard_armed=false;
        }catch(...){
            if(a.guard_armed){destroy_native_singleton_guard_00411ee0(&a.guard);a.guard_armed=false;}
            throw;
        }
        a.result=publication;a.phase=Op::Phase::complete;
        return static_cast<NativeSamplerLoaderSingletonStorage*>(a.result);
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
void* create_native_sampler_factory_resource_00b1b810(const void* name,const void*,
    void* volatile& manager,void* volatile& registry,const NativeResourceRegistryLookupContext& context,Op& a) {
    begin(a,0x00b1b810,manager,registry);a.source_name=name;a.lookup_context=&context;
    try {
        a.native_site=0x00b1b810;a.registry=get_native_resource_registry_00b1b730(manager,registry);
        a.native_site=0x00b1b81c;a.result=create_native_registered_resource_00b19e90(a.registry,name,context);
        a.phase=Op::Phase::complete;return a.result;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
} // namespace bsp
