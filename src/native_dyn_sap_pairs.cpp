#include "bsp/native_dyn_sap_pairs.hpp"

namespace bsp {
namespace {
using U=std::uint32_t;
static_assert(sizeof(void*)==4);
U address(const void* p) noexcept{return reinterpret_cast<U>(p);}
void* pointer(U p) noexcept{return reinterpret_cast<void*>(p);}
void* at(const void* p,U n=0) noexcept{return pointer(address(p)+n);}
volatile U& word(const void* p,U n=0) noexcept{return *static_cast<volatile U*>(at(p,n));}
std::int32_t signed_word(const void* p,U n) noexcept{return static_cast<std::int32_t>(word(p,n));}
void* ptr(const void* p,U n=0) noexcept{return pointer(word(p,n));}
void ptr(void* p,U n,void* q) noexcept{word(p,n)=address(q);}
void release(void* p,U site,const AvoidZoneDynHullMemory& m,NativeDynSapPairCalls& c,NativeDynSapLifetimeProgress& o){o.native_site=site;o.cursor=p;c.sap_free_00bf6989(p,m);}
void remove_reference(void* proxy,void* pair,U site,const AvoidZoneDynHullMemory& m,NativeDynSapPairCalls& c,NativeDynSapLifetimeProgress& o){
    const auto count=signed_word(proxy,0x40);if(count<=0)return;U cursor=word(proxy,0x3c);std::int32_t index=0;
    while(index<count){if(ptr(pointer(cursor))==pair){o.native_site=site;remove_native_dyn_sap_pair_reference_00c40d80(proxy,static_cast<U>(index),m,c,o);return;}++index;cursor+=4;}
}
}
void* NativeDynSapPairCalls::sap_malloc_00bf9f1a(U n,const AvoidZoneDynHullMemory& m){return m.allocate(m.context,n);}
void* find_native_dyn_sap_pair_00c37300(void* eax_proxy,void* stack_proxy) noexcept{
    const auto stack_count=signed_word(stack_proxy,0x40),eax_count=signed_word(eax_proxy,0x40);
    const bool shorter=stack_count<eax_count;const auto count=shorter?stack_count:eax_count;if(count<=0)return nullptr;
    void* other=shorter?eax_proxy:stack_proxy;void* data=ptr(shorter?stack_proxy:eax_proxy,0x3c);U cursor=address(data);std::int32_t index=0;
    while(index<count){void* pair=ptr(pointer(cursor));if(ptr(pair)==other||ptr(pair,4)==other)return ptr(at(data,static_cast<U>(index)*4));++index;cursor+=4;}return nullptr;
}
void append_native_dyn_sap_pair_00c37290(void* proxy,void* pair,const AvoidZoneDynHullMemory& m,NativeDynSapPairCalls& c,NativeDynSapLifetimeProgress& o){
    o.proxy=proxy;o.pair=pair;
    if(word(proxy,0x40)==word(proxy,0x44)){
        const U capacity=word(proxy,0x44)*2+2;word(proxy,0x44)=capacity;o.native_site=0xc372a5;void* grown=c.sap_allocate_00bf55be(capacity*4,m);
        U index=0;void* output=grown;while(index<word(proxy,0x40)){if(output)word(output)=word(at(ptr(proxy,0x3c),index*4));++index;output=at(output,4);}
        if(void* old=ptr(proxy,0x3c))release(old,0xc372d7,m,c,o);ptr(proxy,0x3c,grown);
    }
    void* output=at(ptr(proxy,0x3c),word(proxy,0x40)*4);if(output)ptr(output,0,pair);word(proxy,0x40)=word(proxy,0x40)+1;
}
void create_native_dyn_sap_pair_00c3ffe0(void* manager,void* first,void* second,const AvoidZoneDynHullMemory& m,NativeDynSapPairCalls& c,NativeDynSapLifetimeProgress& o){
    o.manager=manager;o.proxy=first;o.native_site=0xc3ffec;if(find_native_dyn_sap_pair_00c37300(second,first))return;
    if(!ptr(manager,0xac)){
        o.native_site=0xc4000c;void* page=c.sap_malloc_00bf9f1a(16000,m);for(U i=0;i<999;++i)ptr(page,i*16+12,at(page,(i+1)*16));word(page,0x3e7c)=0;
        const U capacity=word(manager,0xa8);const bool grow=word(manager,0xa4)==capacity;ptr(manager,0xac,page);
        if(grow){const U next_capacity=capacity*2+2;word(manager,0xa8)=next_capacity;o.native_site=0xc40056;void* grown=c.sap_allocate_00bf55be(next_capacity*4,m);
            U index=0;void* output=grown;while(index<word(manager,0xa4)){if(output)word(output)=word(at(ptr(manager,0xa0),index*4));++index;output=at(output,4);}
            if(void* old=ptr(manager,0xa0))release(old,0xc40098,m,c,o);ptr(manager,0xa0,grown);
        }
        void* output=at(ptr(manager,0xa0),word(manager,0xa4)*4);if(output)ptr(output,0,page);word(manager,0xa4)=word(manager,0xa4)+1;
    }
    void* pair=ptr(manager,0xac);o.pair=pair;void* next=ptr(pair,12);word(manager,0xd0)=word(manager,0xd0)+1;
    const bool ordered=address(first)<address(second);ptr(manager,0xac,next);ptr(pair,12,at(manager,0xc0));ptr(pair,8,ptr(manager,0xc8));ptr(ptr(manager,0xc8),12,pair);ptr(manager,0xc8,pair);
    if(ordered){ptr(pair,0,first);ptr(pair,4,second);o.native_site=0xc40107;}else{ptr(pair,4,first);ptr(pair,0,second);o.native_site=0xc40120;}
    append_native_dyn_sap_pair_00c37290(first,pair,m,c,o);o.native_site=ordered?0xc4010f:0xc40128;append_native_dyn_sap_pair_00c37290(second,pair,m,c,o);
}
void erase_native_dyn_sap_pair_00c4bcd0(void* manager,void* first,void* second,const AvoidZoneDynHullMemory& m,NativeDynSapPairCalls& c,NativeDynSapLifetimeProgress& o){
    o.manager=manager;o.proxy=first;o.native_site=0xc4bcdc;void* pair=find_native_dyn_sap_pair_00c37300(second,first);o.pair=pair;if(!pair)return;
    remove_reference(first,pair,0xc4bd04,m,c,o);remove_reference(second,pair,0xc4bd28,m,c,o);
    ptr(ptr(pair,12),8,ptr(pair,8));ptr(ptr(pair,8),12,ptr(pair,12));ptr(pair,12,ptr(manager,0xac));word(manager,0xd0)=word(manager,0xd0)-1;ptr(manager,0xac,pair);
}
U count_native_dyn_sap_pairs_00c32b30(const void* manager) noexcept{return word(manager,0xd0);}
void* first_native_dyn_sap_pair_00c32b10(const void* manager) noexcept{return word(manager,0xd0)?ptr(manager,0xbc):nullptr;}
void* next_native_dyn_sap_pair_00c32af0(const void* manager,const void* pair) noexcept{void* next=ptr(pair,12);return next==at(manager,0xc0)?nullptr:next;}
} // namespace bsp
