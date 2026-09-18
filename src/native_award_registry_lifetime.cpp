#include "bsp/native_award_registry_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;using Progress=NativeAwardRegistryLifetimeProgress;using Context=NativeGameProfileLifetimeContext;
static_assert(sizeof(void*)==4);
void* at(void* p,U n=0) noexcept{return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
U word(void* p,U n=0) noexcept{U v;std::memcpy(&v,at(p,n),4);return v;}
void word(void* p,U n,U v) noexcept{std::memcpy(at(p,n),&v,4);}
void* pointer(void* p,U n=0) noexcept{return reinterpret_cast<void*>(word(p,n));}
void release(void* p,U offset,U getter,U put,Context& c,Progress& o){
    if(void* data=pointer(p,offset+4)){U bytes=word(p,offset)+1;o.cursor=at(p,offset);o.native_site=getter;
        auto* pool=c.calls.profile_pool_00419cc0(c.strings);o.native_site=put;c.calls.profile_return_00bd1510(pool,data,bytes,c.strings);}
}
}
NativeAwardRegistryLifetimeOperation::~NativeAwardRegistryLifetimeOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void NativeAwardRegistryLifetimeOperation::acknowledge_diagnostic_cleanup() noexcept{if(phase==Phase::failed)phase=Phase::diagnostic_retired;}
void destroy_native_award_index_list_00487110(void* p,Context& c,Progress& o){
    void* head=pointer(p,4);void* cursor=pointer(head);word(head,0,reinterpret_cast<U>(head));
    head=pointer(p,4);word(head,4,reinterpret_cast<U>(head));const bool nonempty=cursor!=pointer(p,4);word(p,8,0);
    if(nonempty)for(;;){o.cursor=cursor;void* next=pointer(cursor);o.native_site=0x00487133;c.calls.free_00bf65ac(cursor);const bool more=next!=pointer(p,4);cursor=next;if(!more)break;}
    o.native_site=0x00487147;c.calls.free_00bf65ac(pointer(p,4));word(p,4,0);
}
void destroy_native_award_index_list_thunk_00489ca0(void* p,Context& c,Progress& o){destroy_native_award_index_list_00487110(p,c,o);}
void destroy_native_award_record_00501fa0(void* p,Context& c,Progress& o){
    o.record=p;o.record_unwind=3;
    if(void* buffer=pointer(p,0x48)){o.native_site=0x00501fd0;c.calls.free_00bf65ac(buffer);}
    word(p,0x48,0);word(p,0x4c,0);word(p,0x50,0);o.native_site=0x00501fe4;destroy_native_award_index_list_00487110(at(p,0x30),c,o);
    release(p,0x28,0x00501ffa,0x00502001,c,o);o.record_unwind=2;release(p,0x1c,0x0050201c,0x00502023,c,o);
    o.record_unwind=1;release(p,0x14,0x0050203e,0x00502045,c,o);o.record_unwind=0;release(p,0xc,0x0050205f,0x00502066,c,o);
    o.record_unwind=-1;release(p,0,0x00502083,0x0050208a,c,o);
}
void erase_native_award_registry_subtree_006b9120(void* tree,void* node,Context& c,Progress& o){
    while(*static_cast<std::uint8_t*>(at(node,0x69))==0){
        o.node=node;o.native_site=0x006b914d;erase_native_award_registry_subtree_006b9120(tree,pointer(node,8),c,o);
        void* left=pointer(node);o.node=node;o.node_unwind=0;o.native_site=0x006b9166;destroy_native_award_record_00501fa0(at(node,0x14),c,o);
        o.node_unwind=-1;release(node,0xc,0x006b9183,0x006b918a,c,o);o.cursor=node;o.native_site=0x006b9190;c.calls.free_00bf65ac(node);node=left;
    }
}
void* clear_native_award_registry_full_range_006b91f0(void* tree,void* output,void* first_owner,void* first,void* last_owner,void* last,Context& c,Progress& o){
    if(!tree||first_owner!=tree||last_owner!=tree||first!=pointer(pointer(tree,4))||last!=pointer(tree,4))
        throw std::invalid_argument("award tree adapter requires its current full range");
    o.native_site=0x006b923a;erase_native_award_registry_subtree_006b9120(tree,pointer(pointer(tree,4),4),c,o);
    void* head=pointer(tree,4);word(head,4,reinterpret_cast<U>(head));head=pointer(tree,4);word(tree,8,0);word(head,0,reinterpret_cast<U>(head));
    head=pointer(tree,4);word(head,8,reinterpret_cast<U>(head));head=pointer(tree,4);void* first_now=pointer(head);
    word(output,0,reinterpret_cast<U>(tree));word(output,4,reinterpret_cast<U>(first_now));return output;
}
void destroy_native_award_registry_006b9380(void* p,Context& c,NativeAwardRegistryLifetimeOperation& o){
    using Op=NativeAwardRegistryLifetimeOperation;if(o.phase!=Op::Phase::fresh)throw std::logic_error("native award registry destruction is one-shot");
    o.phase=Op::Phase::running;o.owner=p;o.context=&c;
    try{void* head=pointer(p,0x14);void* first=pointer(head);void* tree=at(p,0x10);o.registry_unwind=0;o.native_site=0x006b93ba;
        clear_native_award_registry_full_range_006b91f0(tree,o.iterator_output,tree,first,tree,head,c,o);
        o.native_site=0x006b93c3;c.calls.free_00bf65ac(pointer(tree,4));word(tree,4,0);word(tree,8,0);
        void* begin=pointer(p,4);o.registry_unwind=-1;
        if(begin){void* end=pointer(p,8);o.native_site=0x006b93e9;c.calls.profile_call_00432050(begin,end,c);o.native_site=0x006b93f2;c.calls.free_00bf65ac(pointer(p,4));}
        word(p,4,0);word(p,8,0);word(p,0xc,0);o.phase=Op::Phase::complete;
    }catch(...){o.phase=Op::Phase::failed;throw;}
}
} // namespace bsp
