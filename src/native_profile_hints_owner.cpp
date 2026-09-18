#include "bsp/native_profile_hints_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native profile hints ownership requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;using Op=NativeProfileHintsOwnerOperation;
void* at(void* p,U n=0) noexcept{return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
template<class T=U>T get(void* p,U n=0) noexcept{T v;std::memcpy(&v,at(p,n),sizeof v);return v;}
void put(void* p,U n,U v) noexcept{std::memcpy(at(p,n),&v,4);}
volatile U& depth(void* p) noexcept{return *static_cast<volatile U*>(at(p,0x18));}
void begin(Op& op){if(op.phase!=Op::Phase::fresh)throw std::logic_error("native profile hints operation cannot replay");op.phase=Op::Phase::running;}
void* construct(void* p,NativeProfileHintsOwnerContext& c,Op& op){
    op.owner=p;
    put(p,0,0xce3a44);put(p,8,0);put(p,0xc,0x32);put(p,0x10,0);put(p,0x24,0);
    put(op.temporary,0,0);put(op.temporary,4,0);op.native_site=0x42629d;
    resize_native_string_header_0041dd40(op.temporary,c.strings,10,true);
    void* const data=get<void*>(op.temporary,4);op.captured_temporary_data=data;
    if(data){
        const U size=get(op.temporary)+1;op.captured_temporary_bytes=size;
        op.native_site=0x4262b9;std::memcpy(data,c.initialization_label_00ce3a38,size);
        op.native_site=0x4262c5;auto* const pool=native_string_pool_get_or_create_00419cc0(
            c.strings.actual_published_01090aa8,c.strings.actual_manager_publication_01090aa0);
        op.native_site=0x4262cc;return_native_string_pool_00bd1510(pool,data,size,c.strings.actual_small_returns_disabled_01090aa4);
        op.temporary_returned=true;
    }
    put(p,0x38,0);put(p,0x3c,0);put(p,0x40,0);put(p,0x44,0);put(p,0x48,0);put(p,0x30,0xffffffff);
    return p;
}
}
void* NativeProfileHintsOwnerCalls::manager_00415350(NativeStringRawPoolContext& c){return get_native_singleton_manager_00415350(c.actual_manager_publication_01090aa0);}
void* NativeProfileHintsOwnerCalls::allocate_00bf681b(){return singleton_lifetime_allocate({SingletonAllocationKind::object,0x50,0x50});}
void NativeProfileHintsOwnerCalls::free_00bf65ac(void* p) noexcept{singleton_lifetime_free(p);}
void NativeProfileHintsOwnerCalls::register_00bd0c30(void* p,void* owner){register_native_singleton_object_00bd0c30(p,nullptr,owner);}
void NativeProfileHintsOwnerCalls::enter_section(void* p){EnterCriticalSection(static_cast<CRITICAL_SECTION*>(p));}
void NativeProfileHintsOwnerCalls::leave_section(void* p){LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(p));}
Op::~NativeProfileHintsOwnerOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void Op::acknowledge_diagnostic_cleanup() noexcept{phase=Phase::diagnostic_retired;}
void* construct_native_profile_hints_owner_00426250(void* p,NativeProfileHintsOwnerContext& c,Op& op){
    begin(op);
    try{op.result=construct(p,c,op);op.phase=Op::Phase::complete;return op.result;}
    catch(...){op.phase=Op::Phase::failed;throw;}
}
void destroy_native_profile_hints_owner_00426300(void* p,NativeProfileHintsOwnerContext& c) noexcept{
    c.actual_publication_00e17664=nullptr;put(p,0,0xce3818);
}
void* delete_native_profile_hints_owner_00426320(void* p,U flags,NativeProfileHintsOwnerContext& c) noexcept{
    const bool release=(flags&1)!=0;destroy_native_profile_hints_owner_00426300(p,c);
    if(release)c.calls.free_00bf65ac(p);return p;
}
void* get_native_profile_hints_owner_004c1e90(NativeProfileHintsOwnerContext& c,Op& op){
    begin(op);
    try{
        if(void* const initial=c.actual_publication_00e17664){op.result=initial;op.phase=Op::Phase::complete;return initial;}
        op.native_site=0x4c1eb6;void* const first_manager=c.calls.manager_00415350(c.strings);
        void* const section=get<void*>(first_manager,0x10);op.guard[0]=0xce37fc;op.guard[1]=reinterpret_cast<U>(section);
        if(section){op.native_site=0x4c1ecf;c.calls.enter_section(section);op.guard_held=true;depth(section)=depth(section)+1;}
        op.unwind_state=0;
        if(!c.actual_publication_00e17664){
            op.native_site=0x4c1eec;void* const allocation=c.calls.allocate_00bf681b();op.allocation=allocation;op.unwind_state=1;
            void* const result=allocation?construct(allocation,c,op):nullptr;
            op.unwind_state=0;c.actual_publication_00e17664=result;op.owner_published=true;
            op.native_site=0x4c1f16;void* const manager=c.calls.manager_00415350(c.strings);
            void* const current=c.actual_publication_00e17664;op.registered_owner=current;op.native_site=0x4c1f24;
            c.calls.register_00bd0c30(manager,current);op.registered=true;
        }
        if(section){depth(section)=depth(section)-1;op.guard_depth_restored=true;op.native_site=0x4c1f32;c.calls.leave_section(section);op.guard_held=false;}
        op.result=c.actual_publication_00e17664;op.phase=Op::Phase::complete;return op.result;
    }catch(...){op.phase=Op::Phase::failed;throw;}
}
} // namespace bsp
