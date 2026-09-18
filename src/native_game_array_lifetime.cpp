#include "bsp/native_game_array_lifetime.hpp"
#include "bsp/native_string.hpp"
#include "bsp/observer_lifetime.hpp"
#include <Windows.h>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
using Stage=NativeGameArrayLifetimeStage;
static_assert(sizeof(void*)==4);
void* at(void* p,U n) noexcept {return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
U word(void* p,U n) noexcept {U v;std::memcpy(&v,at(p,n),4);return v;}
void word(void* p,U n,U v) noexcept {std::memcpy(at(p,n),&v,4);}
void* pointer(void* p,U n) noexcept {return reinterpret_cast<void*>(word(p,n));}
void list(void* p,NativeGameArrayLifetimeCalls& calls,Stage& stage,U node_site,U head_site) {
    void* head=pointer(p,4);void* cursor=pointer(head,0);
    word(head,0,reinterpret_cast<U>(head));head=pointer(p,4);word(head,4,reinterpret_cast<U>(head));
    // CMP precedes the count store; the back edge compares against the current
    // head after each free callback. Do not capture one sentinel for the loop.
    const bool nonempty=cursor!=pointer(p,4);word(p,8,0);
    if(nonempty)for(;;){
        void* next=pointer(cursor,0);stage.native_site=node_site;calls.free_00bf65ac(cursor);
        const bool more=next!=pointer(p,4);cursor=next;if(!more)break;
    }
    stage.native_site=head_site;calls.free_00bf65ac(pointer(p,4));word(p,4,0);
}
}
void NativeGameArrayLifetimeCalls::free_00bf65ac(void* p){::operator delete(p);}
NativeGameArrayLifetimeOperation::~NativeGameArrayLifetimeOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void NativeGameArrayLifetimeOperation::acknowledge_diagnostic_cleanup() noexcept {if(phase==Phase::failed)phase=Phase::diagnostic_retired;}
void destroy_native_game_vector_004cafd0(void* p,NativeGameArrayLifetimeCalls& calls,Stage& stage){
    if(void* begin=pointer(p,4)){stage.native_site=0x004cafdb;calls.free_00bf65ac(begin);}
    word(p,4,0);word(p,8,0);word(p,0xc,0);
}
void destroy_native_game_list_004c1990(void* p,NativeGameArrayLifetimeCalls& calls,Stage& stage){list(p,calls,stage,0x004c19b3,0x004c19c7);}
void destroy_native_game_list_thunk_004c4600(void* p,NativeGameArrayLifetimeCalls& calls,Stage& stage){destroy_native_game_list_004c1990(p,calls,stage);}
void destroy_native_game_participant_pairs_004c5860(void* p,NativeGameArrayLifetimeCalls& calls,Stage& stage){list(p,calls,stage,0x004c5883,0x004c5897);}
void release_native_game_participant_references_007f8180(void* p,NativeGameArrayLifetimeCalls& calls,Stage& stage){
    for(U i=0;i<3;++i){void* cell=at(p,0x104+i*4);if(void* captured=pointer(cell,0)){
        stage.native_site=0x007f819f;
        if(::InterlockedDecrement(static_cast<volatile LONG*>(at(captured,4)))==0){
            stage.native_site=0x007f81ab;calls.virtual_terminal(captured);
        }
        word(cell,0,0);
    }}
}
void destroy_native_game_participant_observer_004b7ef0(void* p,NativeObserverLifetime& observers,Stage& stage){
    word(p,0,0x00ce74fc);stage.unwind_state=0;
    auto& callback=*static_cast<NativeObserverOwnerStorage*>(p);
    if(void* first=pointer(p,0x14)){
        stage.native_site=0x004b7f24;observers.unregister_pair_006952a0(*static_cast<NativeObserverOwnerStorage*>(first),callback);
    }
    stage.unwind_state=-1;stage.native_site=0x004b7f33;observers.destroy_callback_owner_00695870(callback);
}
void destroy_native_game_participant_004cb2f0(void* p,NativeGameArrayLifetimeContext& context,Stage& stage){
    stage.owner=p;word(p,0,0x00ce7794);stage.unwind_state=2;stage.native_site=0x004cb31b;
    release_native_game_participant_references_007f8180(p,context.calls,stage);
    stage.unwind_state=1;stage.native_site=0x004cb335;
    for(U remaining=3;remaining!=0;){--remaining;destroy_native_string_header_0041dd20(at(p,0xec+remaining*8),context.strings);}
    stage.native_site=0x004cb340;destroy_native_game_participant_pairs_004c5860(at(p,0x98),context.calls,stage);
    stage.unwind_state=-1;stage.native_site=0x004cb350;
    destroy_native_game_participant_observer_004b7ef0(at(p,0x38),context.observers,stage);
}
void destroy_native_game_array_00bf7c6e(void* p,U stride,U count,U destructor,
    NativeGameArrayLifetimeContext& context,NativeGameArrayLifetimeOperation& operation){
    using Op=NativeGameArrayLifetimeOperation;
    if(operation.phase!=Op::Phase::fresh)throw std::logic_error("game array destruction is one-shot");
    const bool vector=stride==0x10&&destructor==0x004cafd0;
    const bool participant=stride==0x118&&destructor==0x004cb2f0;
    const bool list_array=stride==0xc&&destructor==0x004c4600;
    if(!vector&&!participant&&!list_array)throw std::invalid_argument("unsupported native game-array destructor contract");
    operation.base=p;operation.stride=stride;operation.count=count;operation.destructor=destructor;operation.phase=Op::Phase::running;
    try{
        while(operation.completed<count){
            void* element=at(p,(count-operation.completed-1)*stride);auto& stage=operation.element;
            stage.owner=element;stage.native_site=destructor;stage.unwind_state=-1;
            if(vector)destroy_native_game_vector_004cafd0(element,context.calls,stage);
            else if(participant)destroy_native_game_participant_004cb2f0(element,context,stage);
            else destroy_native_game_list_thunk_004c4600(element,context.calls,stage);
            ++operation.completed;
        }
        operation.phase=Op::Phase::complete;
    }catch(...){operation.phase=Op::Phase::failed;throw;}
}
} // namespace bsp
