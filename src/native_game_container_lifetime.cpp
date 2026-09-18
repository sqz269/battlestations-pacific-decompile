#include "bsp/native_game_container_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <Windows.h>
#include <cstdlib>
#include <cstring>
#include <new>

namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;using Progress=NativeGameContainerLifetimeProgress;using Calls=NativeGameContainerLifetimeCalls;
static_assert(sizeof(void*)==4&&sizeof(LONG)==4);
void* at(void* p,U n=0) noexcept{return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
U word(void* p,U n=0) noexcept{U v;std::memcpy(&v,at(p,n),4);return v;}
void word(void* p,U n,U v) noexcept{std::memcpy(at(p,n),&v,4);}
void* pointer(void* p,U n=0) noexcept{return reinterpret_cast<void*>(word(p,n));}
I signed_bits(U v) noexcept{I r;std::memcpy(&r,&v,4);return r;}
void begin(void* p,Progress& o) noexcept{o.owner=p;o.cursor=nullptr;o.replacement_allocation=nullptr;o.index=0;o.unwind_state=-1;}
void unlink(void* p,void* node) noexcept{
    if(void* prev=pointer(node))word(prev,4,word(node,4));else word(p,4,word(node,4));
    if(pointer(node,4)){void* next=pointer(node,4);word(next,0,word(node));}else word(p,8,word(node));
    word(p,0,word(p)-1);
}
void clear_links(void* p,Calls& calls,Progress& o,U free_site,bool payload){
    begin(p,o);
    while(word(p)!=0){void* node=pointer(p,4);o.cursor=node;
        if(payload)if(void* value=pointer(node,8)){o.native_site=0x004bf950;calls.virtual_scalar(value,0,1);word(node,8,0);}
        unlink(p,node);o.native_site=free_site;calls.free_00bf65ac(node);
    }
}
void release_cell(void* cell,Calls& calls,Progress& o,U atomic_site,U terminal_site){
    o.cursor=cell;
    if(void* value=pointer(cell)){
        o.native_site=atomic_site;
        if(calls.interlocked_decrement(static_cast<volatile I*>(at(value,4)))==0){o.native_site=terminal_site;calls.virtual_terminal(value);}
        word(cell,0,0);
    }
}
}
void* Calls::allocate_00bf55be(U n){return singleton_lifetime_allocate({SingletonAllocationKind::object,n,n});}
void Calls::free_00bf65ac(void* p){::operator delete(p);}
void Calls::free_00bf6989(void* p){std::free(p);}
I Calls::interlocked_increment(volatile I* p){return InterlockedIncrement(reinterpret_cast<volatile LONG*>(p));}
I Calls::interlocked_decrement(volatile I* p){return InterlockedDecrement(reinterpret_cast<volatile LONG*>(p));}
void clear_native_game_unit_links_004bf8e0(void* p,Calls& c,Progress& o){clear_links(p,c,o,0x004bf917,false);}
void clear_native_game_scene_links_004c2ce0(void* p,Calls& c,Progress& o){clear_links(p,c,o,0x004c2d17,false);}
void destroy_native_game_scene_records_004bf930(void* p,Calls& c,Progress& o){clear_links(p,c,o,0x004bf985,true);}
void destroy_native_game_pointer_list_004c4b40(void* p,Calls& c,Progress& o){
    begin(p,o);void* head=pointer(p,4);void* cursor=pointer(head);word(head,0,reinterpret_cast<U>(head));
    head=pointer(p,4);word(head,4,reinterpret_cast<U>(head));const bool nonempty=cursor!=pointer(p,4);word(p,8,0);
    if(nonempty)for(;;){o.cursor=cursor;void* next=pointer(cursor);o.native_site=0x004c4b63;c.free_00bf65ac(cursor);const bool more=next!=pointer(p,4);cursor=next;if(!more)break;}
    o.native_site=0x004c4b77;c.free_00bf65ac(pointer(p,4));word(p,4,0);
}
void destroy_native_game_pointer_list_thunk_004c8180(void* p,Calls& c,Progress& o){destroy_native_game_pointer_list_004c4b40(p,c,o);}
void clear_native_game_small_string_nodes_004cf3f0(void* p,Calls& c,Progress& o){
    begin(p,o);void* head=pointer(p,4);void* cursor=pointer(head);word(head,0,reinterpret_cast<U>(head));
    head=pointer(p,4);word(head,4,reinterpret_cast<U>(head));const bool nonempty=cursor!=pointer(p,4);word(p,8,0);
    if(nonempty)for(;;){
        o.cursor=cursor;const bool large=word(cursor,0x20)>=16;void* next=pointer(cursor);
        if(large){o.native_site=0x004cf41c;c.free_00bf65ac(pointer(cursor,0xc));}
        word(cursor,0x20,15);word(cursor,0x1c,0);*static_cast<volatile std::uint8_t*>(at(cursor,0xc))=0;
        o.native_site=0x004cf432;c.free_00bf65ac(cursor);const bool more=next!=pointer(p,4);cursor=next;if(!more)break;
    }
}
void destroy_native_game_pointer_blocks_004c4a50(void* p,Calls& c,Progress& o){
    begin(p,o);
    while(word(p,0x10)!=0){U value=word(p,0x10);if(value!=0){--value;word(p,0x10,value);if(value==0)word(p,0xc,0);}}
    U count=word(p,8);
    while(count!=0){void* base=pointer(p,4);--count;o.index=count;o.cursor=at(base,count*4);if(pointer(o.cursor)){void* value=pointer(o.cursor);o.native_site=0x004c4a92;c.free_00bf65ac(value);}}
    if(void* base=pointer(p,4)){o.native_site=0x004c4aa7;c.free_00bf65ac(base);}word(p,4,0);word(p,8,0);
}
void reserve_native_game_reference_slots_004c86a0(void* p,U requested,Calls& c,Progress& o){
    begin(p,o);if(signed_bits(requested)<1)requested=1;
    if(signed_bits(word(p,8))>=signed_bits(requested))return;
    o.native_site=0x004c86e1;void* replacement=c.allocate_00bf55be(requested*4);o.replacement_allocation=replacement;
    U index=0;void* cursor=replacement;
    while(signed_bits(index)<signed_bits(word(p,4))){
        o.index=index;o.cursor=cursor;
        if(cursor){void* old=at(pointer(p),index*4);word(cursor,0,0);if(void* value=pointer(old)){word(cursor,0,reinterpret_cast<U>(value));o.native_site=0x004c871b;c.interlocked_increment(static_cast<volatile I*>(at(value,4)));}}
        ++index;cursor=at(cursor,4);o.unwind_state=-1;
    }
    index=0;
    while(signed_bits(index)<signed_bits(word(p,4))){o.index=index;void* cell=at(pointer(p),index*4);release_cell(cell,c,o,0x004c8747,0x004c8757);++index;}
    o.native_site=0x004c876a;c.free_00bf6989(pointer(p));word(p,0,reinterpret_cast<U>(replacement));word(p,8,requested);
}
void resize_native_game_reference_slots_004cb220(void* p,U requested,Calls& c,Progress& o){
    begin(p,o);
    if(signed_bits(requested)>signed_bits(word(p,8))){o.native_site=0x004cb22e;reserve_native_game_reference_slots_004c86a0(p,requested,c,o);}
    U index=word(p,4);
    while(signed_bits(index)<signed_bits(requested)){void* cell=at(pointer(p),index*4);if(cell)word(cell,0,0);++index;}
    while(signed_bits(requested)<signed_bits(word(p,4))){word(p,4,word(p,4)-1);o.index=word(p,4);void* cell=at(pointer(p),o.index*4);release_cell(cell,c,o,0x004cb277,0x004cb287);}
    word(p,4,requested);
}
} // namespace bsp
