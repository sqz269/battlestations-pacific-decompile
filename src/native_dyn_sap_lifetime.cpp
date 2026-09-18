#include "bsp/native_dyn_sap_lifetime.hpp"

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
void release69(void* p,U site,const AvoidZoneDynHullMemory& m,NativeDynSapLifetimeCalls& c,NativeDynSapLifetimeProgress& o){o.native_site=site;o.cursor=p;c.sap_free_00bf6989(p,m);}
void pages(void* pool,U page_site,U vector_site,const AvoidZoneDynHullMemory& m,NativeDynSapLifetimeCalls& c,NativeDynSapLifetimeProgress& o){
    U index=0;while(index<word(pool,4)){o.native_site=page_site;o.cursor=at(ptr(pool),index*4);c.sap_free_00bf9dc8(ptr(o.cursor),m);++index;}
    if(void* p=ptr(pool))release69(p,vector_site,m,c,o);
}
void endpoint(void* manager,void* proxy,U offset,U free_offset,U count_offset,bool publish_before_count){
    void* p=ptr(proxy,offset);ptr(ptr(p,12),8,ptr(p,8));ptr(ptr(p,8),12,ptr(p,12));ptr(p,12,ptr(manager,free_offset));
    if(publish_before_count)ptr(manager,free_offset,p);word(manager,count_offset)=word(manager,count_offset)-1;
    if(!publish_before_count)ptr(manager,free_offset,p);
}
}
void* NativeDynSapLifetimeCalls::sap_allocate_00bf55be(U n,const AvoidZoneDynHullMemory& m){return m.allocate(m.context,n);}
void NativeDynSapLifetimeCalls::sap_free_00bf65ac(void* p,const AvoidZoneDynHullMemory& m){m.release(m.context,p);}
void NativeDynSapLifetimeCalls::sap_free_00bf6989(void* p,const AvoidZoneDynHullMemory& m){m.release(m.context,p);}
void NativeDynSapLifetimeCalls::sap_free_00bf9dc8(void* p,const AvoidZoneDynHullMemory& m){m.release(m.context,p);}
void remove_native_dyn_sap_pair_reference_00c40d80(void* proxy,U index,const AvoidZoneDynHullMemory& m,NativeDynSapLifetimeCalls& c,NativeDynSapLifetimeProgress& o){
    void* data=ptr(proxy,0x3c);const U last=word(at(data,word(proxy,0x40)*4-4));word(at(data,index*4))=last;
    const U count=word(proxy,0x40)-1;
    if(count>word(proxy,0x40)){
        if(count>word(proxy,0x44)){word(proxy,0x44)=count;o.native_site=0xc40db0;void* grown=c.sap_allocate_00bf55be(count*4,m);U i=0;void* output=grown;
            while(i<word(proxy,0x40)){if(output)word(output)=word(at(ptr(proxy,0x3c),i*4));++i;output=at(output,4);}
            if(void* old=ptr(proxy,0x3c))release69(old,0xc40de2,m,c,o);ptr(proxy,0x3c,grown);
        }
        U i=word(proxy,0x40);while(i<count){void* output=at(ptr(proxy,0x3c),i*4);if(output)word(output)=0;++i;}
    }
    word(proxy,0x40)=count;
}
void clear_native_dyn_sap_proxy_pairs_00c4bc50(void* manager,void* proxy,const AvoidZoneDynHullMemory& m,NativeDynSapLifetimeCalls& c,NativeDynSapLifetimeProgress& o){
    o.manager=manager;o.proxy=proxy;std::int32_t index=0;
    while(index<signed_word(proxy,0x40)){
        void* pair=ptr(at(ptr(proxy,0x3c),static_cast<U>(index)*4));o.pair=pair;void* peer=ptr(pair);if(peer==proxy)peer=ptr(pair,4);
        const auto peer_count=signed_word(peer,0x40);if(peer_count>0){U cursor=word(peer,0x3c);std::int32_t i=0;
            while(i<peer_count){if(ptr(pointer(cursor))==pair){o.native_site=0xc4bc91;remove_native_dyn_sap_pair_reference_00c40d80(peer,static_cast<U>(i),m,c,o);break;}++i;cursor+=4;}
        }
        ptr(ptr(pair,12),8,ptr(pair,8));ptr(ptr(pair,8),12,ptr(pair,12));ptr(pair,12,ptr(manager,0xac));word(manager,0xd0)=word(manager,0xd0)-1;
        ++index;ptr(manager,0xac,pair);
    }
}
void remove_native_dyn_sap_proxy_00c4c380(void* manager,void* proxy,const AvoidZoneDynHullMemory& m,NativeDynSapLifetimeCalls& c,NativeDynSapLifetimeProgress& o){
    o.manager=manager;o.proxy=proxy;
    if(*static_cast<volatile unsigned char*>(at(proxy,0x38))){
        endpoint(manager,proxy,0x20,0x10,0x34,true);endpoint(manager,proxy,0x2c,0x10,0x34,false);
        endpoint(manager,proxy,0x24,0x44,0x68,false);endpoint(manager,proxy,0x30,0x44,0x68,false);
        endpoint(manager,proxy,0x28,0x78,0x9c,false);endpoint(manager,proxy,0x34,0x78,0x9c,false);
        o.native_site=0xc4c463;clear_native_dyn_sap_proxy_pairs_00c4bc50(manager,proxy,m,c,o);
    }else{
        const U end=word(manager,0x23c)+word(manager,0x240)*4;U cursor=word(manager,0x23c);
        while(cursor!=end&&word(pointer(cursor))!=address(proxy))cursor+=4;
        while(cursor!=word(manager,0x23c)+word(manager,0x240)*4-4){word(pointer(cursor))=word(pointer(cursor+4));cursor+=4;}
        word(manager,0x240)=word(manager,0x240)-1;
    }
    const bool is_static=*static_cast<volatile unsigned char*>(at(proxy,0x1c))!=0;void* data=ptr(proxy,0x3c);
    if(data)release69(data,is_static?0xc4c4d5:0xc4c50f,m,c,o);
    ptr(ptr(proxy,0x4c),0x48,ptr(proxy,0x48));ptr(ptr(proxy,0x48),0x4c,ptr(proxy,0x4c));
    const U free_offset=is_static?0x194:0xe0,count_offset=is_static?0x238:0x184;
    ptr(proxy,0x4c,ptr(manager,free_offset));word(manager,count_offset)=word(manager,count_offset)-1;ptr(manager,free_offset,proxy);
}
void destroy_native_dyn_sap_endpoint_pool_0040b590(void* pool,const AvoidZoneDynHullMemory& m,NativeDynSapLifetimeCalls& c,NativeDynSapLifetimeProgress& o){pages(pool,0x40b5a6,0x40b5bd,m,c,o);}
void destroy_native_dyn_sap_manager_004043f0(void* manager,const AvoidZoneDynHullMemory& m,NativeDynSapLifetimeCalls& c,NativeDynSapLifetimeProgress& o){
    o.manager=manager;o.completed_endpoint_pools=0;
    if(void* pending=ptr(manager,0x23c))release69(pending,0x40441e,m,c,o);
    pages(at(manager,0x188),0x40443a,0x404458,m,c,o);pages(at(manager,0xd4),0x40447a,0x404498,m,c,o);pages(at(manager,0xa0),0x4044ba,0x4044d8,m,c,o);
    // Consumed normal CRT reverse iteration: exactly3 elements,34h stride,
    // ECX element callback0040B590. Original exception cleanup is not supplied.
    o.native_site=0x4044ed;
    for(U remaining=3;remaining;){--remaining;o.cursor=at(manager,4+remaining*0x34);destroy_native_dyn_sap_endpoint_pool_0040b590(o.cursor,m,c,o);++o.completed_endpoint_pools;}
    word(manager)=0x00d7a0e4;
}
void* delete_native_dyn_sap_manager_004043d0(void* manager,U flags,const AvoidZoneDynHullMemory& m,NativeDynSapLifetimeCalls& c,NativeDynSapLifetimeProgress& o){
    o.native_site=0x4043d4;destroy_native_dyn_sap_manager_004043f0(manager,m,c,o);
    if(flags&1){o.native_site=0x4043e1;o.cursor=manager;c.sap_free_00bf65ac(manager,m);}return manager;
}
} // namespace bsp
