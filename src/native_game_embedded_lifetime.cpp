#include "bsp/native_game_embedded_lifetime.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include "bsp/random_threads.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
using Progress=NativeGameEmbeddedLifetimeProgress;
static_assert(sizeof(void*)==4);
void* at(void* p,U n) noexcept{return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
U word(void* p,U n) noexcept{U v;std::memcpy(&v,at(p,n),4);return v;}
void word(void* p,U n,U v) noexcept{std::memcpy(at(p,n),&v,4);}
void* pointer(void* p,U n) noexcept{return reinterpret_cast<void*>(word(p,n));}
void byte(void* p,U n) noexcept{*static_cast<volatile std::uint8_t*>(at(p,n))=0;}
void invalid(NativeGameEmbeddedLifetimeCalls& calls,Progress& state,U site){state.native_site=site;calls.invalid_parameter_00bf6713();}
}
void NativeGameEmbeddedLifetimeCalls::free_00bf65ac(void* p){::operator delete(p);}
void NativeGameEmbeddedLifetimeCalls::free_00bf6989(void* p){std::free(p);}
void NativeGameEmbeddedLifetimeCalls::call_0041cc80(void* p){release_native_tracked_critical_section_0041cc80(static_cast<TrackedCriticalSection**>(p));}
void NativeGameEmbeddedLifetimeCalls::invalid_parameter_00bf6713(){_invalid_parameter_noinfo();}
NativeGameEmbeddedLifetimeOperation::~NativeGameEmbeddedLifetimeOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void NativeGameEmbeddedLifetimeOperation::acknowledge_diagnostic_cleanup() noexcept{if(phase==Phase::failed){entered_section_pending=false;phase=Phase::diagnostic_retired;}}
void disable_native_game_embedded_owners_0076a760(void* p,volatile std::uint8_t& flag) noexcept{
    flag=0;if(void* first=pointer(p,0x188))byte(first,0x94);if(void* second=pointer(p,0x18c))byte(second,0x94);
}
void flush_native_game_peer_records_007849c0(void* p,NativeGameEmbeddedLifetimeCalls& calls,Progress& state){
    state.peer=p;auto* entered=static_cast<TrackedCriticalSection*>(pointer(p,0x44));
    state.entered_section=entered;state.native_site=0x007849d1;
    EnterCriticalSection(&entered->native);word(entered,0x18,word(entered,0x18)+1);state.entered_section_pending=true;
    void* cursor=pointer(pointer(p,0xc),0);void* first_header=at(p,8);state.key_count=0;
    for(;;){
        void* captured_head=pointer(first_header,4);
        if(!first_header)invalid(calls,state,0x007849f0);
        if(cursor==captured_head)break;
        if(!first_header)invalid(calls,state,0x007849fd);
        if(cursor==pointer(first_header,4))invalid(calls,state,0x00784a07);
        const bool at_end=cursor==pointer(first_header,4);
        state.keys[state.key_count]=word(cursor,8);
        if(at_end)invalid(calls,state,0x00784a18);
        cursor=pointer(cursor,0);++state.key_count;
    }
    for(U i=state.key_count;i<8;++i)state.keys[i]=0;
    // The original caches key7 in EBP before walking the second list.
    const U last_key=state.keys[7];void* header=at(p,0x38);cursor=pointer(pointer(header,4),0);
    for(;;){
        void* captured_head=pointer(header,4);if(cursor==captured_head)break;
        state.cursor=cursor;
        if(cursor==pointer(header,4))invalid(calls,state,0x00784a5d);
        const U key=word(pointer(cursor,8),8);
        bool match=false;
        if(key){for(U i=0;i<7;++i)if(key==state.keys[i]){match=true;break;}if(!match)match=key==last_key;}
        if(match){
            if(cursor==pointer(header,4))invalid(calls,state,0x00784a9f);
            void* receiver=pointer(p,4);void* record=pointer(cursor,8);
            state.native_site=0x00784ab4;calls.virtual_04(receiver,record);
        }
        if(cursor==pointer(header,4))invalid(calls,state,0x00784abb);
        cursor=pointer(cursor,0);
    }
    void* head=pointer(header,4);cursor=pointer(head,0);word(head,0,reinterpret_cast<U>(head));
    head=pointer(header,4);word(head,4,reinterpret_cast<U>(head));
    const bool nonempty=cursor!=pointer(header,4);word(header,8,0);
    if(nonempty)for(;;){
        state.cursor=cursor;void* next=pointer(cursor,0);state.native_site=0x00784ae3;calls.free_00bf65ac(cursor);
        const bool more=next!=pointer(header,4);cursor=next;if(!more)break;
    }
    auto* released=static_cast<TrackedCriticalSection*>(pointer(p,0x44));
    word(released,0x18,word(released,0x18)-1);state.native_site=0x00784afe;LeaveCriticalSection(&released->native);
    if(released==entered)state.entered_section_pending=false;
}
void clear_native_game_embedded_entries_0076db20(void* p,NativeGameEmbeddedLifetimeCalls& calls,Progress& state){
    void* preferred=pointer(p,0x188);void* peer=preferred?preferred:pointer(p,0x18c);
    if(peer){if(!preferred)peer=pointer(p,0x18c);state.native_site=0x0076db46;flush_native_game_peer_records_007849c0(peer,calls,state);}
    U count=word(p,0x250);void* begin=pointer(p,0x24c);void* cursor=pointer(p,0x24c);
    if(cursor!=at(begin,count*4))for(;;){
        state.cursor=cursor;
        if(void* entry=pointer(cursor,0)){state.native_site=0x0076db70;calls.virtual_scalar(entry,0,1);}
        count=word(p,0x250);begin=pointer(p,0x24c);cursor=at(cursor,4);
        if(cursor==at(begin,count*4))break;
    }
    word(p,0x250,0);
}
void destroy_native_game_embedded_state_0076f000(void* p,NativeGameEmbeddedLifetimeContext& context,NativeGameEmbeddedLifetimeOperation& operation){
    using Op=NativeGameEmbeddedLifetimeOperation;
    if(operation.phase!=Op::Phase::fresh)throw std::logic_error("embedded game destruction is one-shot");
    operation.owner=p;operation.context=&context;operation.phase=Op::Phase::running;auto& calls=context.calls;
    try{
        word(p,0,0x00d039cc);operation.unwind_state=2;operation.native_site=0x0076f02c;
        clear_native_game_embedded_entries_0076db20(p,calls,operation);
        if(void* second=pointer(p,0x18c)){operation.native_site=0x0076f043;calls.virtual_scalar(second,0,1);word(p,0x18c,0);}
        if(void* first=pointer(p,0x188)){operation.native_site=0x0076f05b;calls.virtual_scalar(first,0,1);word(p,0x188,0);}
        operation.native_site=0x0076f069;calls.call_0041cc80(at(p,0x298));
        if(void* buffer=pointer(p,0x25c)){operation.native_site=0x0076f079;calls.free_00bf6989(buffer);word(p,0x25c,0);}
        if(void* buffer=pointer(p,0x24c)){operation.native_site=0x0076f092;calls.free_00bf6989(buffer);word(p,0x24c,0);}
        if(word(p,0x238)>=16){operation.native_site=0x0076f0b0;calls.free_00bf65ac(pointer(p,0x224));}
        word(p,0x234,0);word(p,0x238,15);byte(p,0x224);operation.phase=Op::Phase::complete;
    }catch(...){operation.phase=Op::Phase::failed;throw;}
}
} // namespace bsp
