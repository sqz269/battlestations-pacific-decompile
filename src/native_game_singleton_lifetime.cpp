#include "bsp/native_game_singleton_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include <Windows.h>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
static_assert(sizeof(void*)==4);
void* at(void* p,U n) noexcept {return static_cast<char*>(p)+n;}
U word(void* p,U n) noexcept {U v;std::memcpy(&v,at(p,n),4);return v;}
void word(void* p,U n,U v) noexcept {std::memcpy(at(p,n),&v,4);}
struct Sites {U entry,get_first,enter,get_second,remove,scalar,leave;};
void run(NativeGameSingletonLifetimeContext& x,NativeGameSingletonLifetimeOperation& o,
    void* volatile& cell,U adjustment,U slot,const Sites& sites) {
    if(o.phase!=NativeGameSingletonLifetimeOperation::Phase::fresh)
        throw std::logic_error("native singleton deletion cannot be replayed");
    o.context=&x;o.publication=&cell;o.native_site=sites.entry;
    o.phase=NativeGameSingletonLifetimeOperation::Phase::running;
    try {
        if(cell) {
            auto& c=x.calls;o.native_site=sites.get_first;
            void* manager=c.singleton_manager_00415350(x.actual_manager_01090aa0);
            void* section=reinterpret_cast<void*>(word(manager,0x10));o.captured_section=section;
            if(section) {
                o.native_site=sites.enter;c.enter_singleton_section(section);
                o.entered_section_pending=true;word(section,0x18,word(section,0x18)+1);
            }
            if(void* captured=cell) {
                // The unadjusted instantiation loads its object only after
                // the getter; the two adjusted instantiations push it first.
                void* object=adjustment?at(captured,adjustment):nullptr;
                o.native_site=sites.get_second;manager=c.singleton_manager_00415350(x.actual_manager_01090aa0);
                if(!adjustment)object=cell;
                o.native_site=sites.remove;c.unregister_singleton_00bcfca0(manager,object);
                if(void* current=cell) {o.native_site=sites.scalar;c.virtual_scalar(current,slot,1);}
                cell=nullptr;
            }
            if(section) {
                word(section,0x18,word(section,0x18)-1);o.native_site=sites.leave;
                c.leave_singleton_section(section);o.entered_section_pending=false;
            }
        }
        o.phase=NativeGameSingletonLifetimeOperation::Phase::complete;
    } catch(...) {o.phase=NativeGameSingletonLifetimeOperation::Phase::failed;throw;}
}
}
void* NativeGameSingletonLifetimeCalls::singleton_manager_00415350(void* volatile& p){return get_native_singleton_manager_00415350(p);}
void NativeGameSingletonLifetimeCalls::unregister_singleton_00bcfca0(void* m,void* p){unregister_native_singleton_object_00bcfca0(m,nullptr,p);}
void NativeGameSingletonLifetimeCalls::enter_singleton_section(void* p){::EnterCriticalSection(static_cast<CRITICAL_SECTION*>(p));}
void NativeGameSingletonLifetimeCalls::leave_singleton_section(void* p){::LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(p));}
NativeGameSingletonLifetimeOperation::~NativeGameSingletonLifetimeOperation(){
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void NativeGameSingletonLifetimeOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::failed)phase=Phase::diagnostic_retired;
}
void delete_native_game_singleton_004c0c30(NativeGameSingletonLifetimeContext& x,NativeGameSingletonLifetimeOperation& o){
    run(x,o,x.publication_00e18e6c,0xc,0xc,{0x4c0c30,0x4c0c52,0x4c0c6b,0x4c0c8a,0x4c0c91,0x4c0ca7,0x4c0cbc});
}
void delete_native_game_singleton_004c0ce0(NativeGameSingletonLifetimeContext& x,NativeGameSingletonLifetimeOperation& o){
    run(x,o,x.publication_00e18d80,8,0xc,{0x4c0ce0,0x4c0d02,0x4c0d1b,0x4c0d3a,0x4c0d41,0x4c0d57,0x4c0d6c});
}
void delete_native_game_singleton_004c0d90(NativeGameSingletonLifetimeContext& x,NativeGameSingletonLifetimeOperation& o){
    run(x,o,x.publication_00f89b34,0,0,{0x4c0d90,0x4c0db2,0x4c0dcb,0x4c0de6,0x4c0df4,0x4c0e09,0x4c0e1e});
}
void native_game_cleanup_noop_008d88f0() noexcept {}
} // namespace bsp
