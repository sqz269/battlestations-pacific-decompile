#include "bsp/native_game_physics_lifetime.hpp"
#include "bsp/dyn_task_manager.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <Windows.h>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
static_assert(sizeof(void*)==4);
U address(const void* p) noexcept{return reinterpret_cast<U>(p);}
void* pointer(U p) noexcept{return reinterpret_cast<void*>(p);}
void* at(const void* p,U n=0) noexcept{return pointer(address(p)+n);}
volatile U& word(const void* p,U n=0) noexcept{return *static_cast<volatile U*>(at(p,n));}
void* ptr(const void* p,U n=0) noexcept{return pointer(word(p,n));}
void ptr(void* p,U n,void* q) noexcept{word(p,n)=address(q);}
void release65(void* p,U site,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){o.native_site=site;o.captured=p;c.physics_free_00bf65ac(p,m);}
void release69(void* p,U site,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){o.native_site=site;o.captured=p;c.physics_free_00bf6989(p,m);}
void release9d(void* p,U site,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){o.native_site=site;o.captured=p;c.physics_free_00bf9dc8(p,m);}
void pages(void* p,U offset,U site,U vector_site,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){
    U i=0;while(i<word(p,offset+4)){o.cursor=at(ptr(p,offset),i*4);release9d(ptr(o.cursor),site,m,c,o);++i;}
    if(void* data=ptr(p,offset))release69(data,vector_site,m,c,o);
}
void active_bodies(void* world,U head_offset,U sentinel_offset,U broadphase_site,U scalar_site,U manifolds_site,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){
    void* body=ptr(world,head_offset);void* sentinel=at(world,sentinel_offset);
    while(body!=sentinel){o.cursor=body;
        if(word(body,0x70)){void* manager=ptr(ptr(ptr(body),0x444),0xac);void* handle=ptr(body,0x60);
            o.native_site=broadphase_site;o.captured=manager;c.physics_broadphase_remove(manager,handle,m,o.sap);word(body,0x50)&=~8u;word(body,0x60)=0;}
        void* attachment=ptr(body,0x70);
        while(attachment){void* next=ptr(attachment,0x208);o.native_site=scalar_site;o.captured=attachment;c.physics_virtual_scalar(attachment,4,1);attachment=next;}
        word(body,0x70)=0;o.native_site=manifolds_site;clear_native_dyn_body_manifolds_00c43aa0(body,m,c,o);body=ptr(body,0x84);
    }
}
void recycle_bodies(void* world,U head_offset,U sentinel_offset,U count_offset,U free_offset,U tail_offset,U pool_start,U repeated_site,U last_site,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){
    if(!word(world,count_offset))return;
    void* body=ptr(world,head_offset);void* sentinel=at(world,sentinel_offset);
    while(ptr(body,0x84)!=sentinel){o.cursor=body;o.native_site=repeated_site;destroy_native_dyn_body_storage_00c43c00(body,m,c,o);body=ptr(body,0x84);}
    o.cursor=body;o.native_site=last_site;destroy_native_dyn_body_storage_00c43c00(body,m,c,o);
    ptr(body,0x84,ptr(world,free_offset));ptr(world,free_offset,ptr(world,head_offset));ptr(world,head_offset,sentinel);ptr(world,tail_offset,at(world,pool_start));word(world,count_offset)=0;
}
}
void* NativeGamePhysicsLifetimeCalls::physics_allocate_00bf55be(U n,const AvoidZoneDynHullMemory& m){return m.allocate(m.context,n);}
void NativeGamePhysicsLifetimeCalls::physics_free_00bf65ac(void* p,const AvoidZoneDynHullMemory& m){m.release(m.context,p);}
void NativeGamePhysicsLifetimeCalls::physics_free_00bf6989(void* p,const AvoidZoneDynHullMemory& m){m.release(m.context,p);}
void NativeGamePhysicsLifetimeCalls::physics_free_00bf9dc8(void* p,const AvoidZoneDynHullMemory& m){m.release(m.context,p);}
void NativeGamePhysicsLifetimeCalls::physics_free_profile_text_00bf65ac(void* p){singleton_lifetime_free(p);}
void NativeGamePhysicsLifetimeCalls::physics_delete_section(void* p){DeleteCriticalSection(static_cast<CRITICAL_SECTION*>(p));}
void NativeGamePhysicsLifetimeCalls::physics_virtual_scalar(void* p,U slot,U flags){
    using Method=void*(__thiscall*)(void*,U);auto method=reinterpret_cast<Method>(word(ptr(p),slot));(void)method(p,flags);
}
void NativeGamePhysicsLifetimeCalls::physics_broadphase_remove(void* p,void* handle,const AvoidZoneDynHullMemory& m,NativeDynSapLifetimeProgress& o){remove_native_dyn_sap_proxy_00c4c380(p,handle,m,*this,o);}
void NativeGamePhysicsLifetimeCalls::physics_delete_broadphase_004043d0(void* p,U flags,const AvoidZoneDynHullMemory& m,NativeDynSapLifetimeProgress& o){delete_native_dyn_sap_manager_004043d0(p,flags,m,*this,o);}
void NativeGamePhysicsLifetimeCalls::physics_destroy_tasks_00c40ff0(void* p,const AvoidZoneDynHullMemory& m){dyn_task_manager_destroy_00c40ff0(*static_cast<DynTaskManagerStorage*>(p),m);}
void remove_native_dyn_manifold_reference_00c37d30(void* body,void* manifold,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){
    const U end=word(body,0x74)+word(body,0x78)*4;U cursor=word(body,0x74);
    while(cursor!=end&&word(pointer(cursor))!=address(manifold))cursor+=4;
    word(pointer(cursor))=word(at(ptr(body,0x74),word(body,0x78)*4-4));
    const U count=word(body,0x78)-1;
    if(count>word(body,0x78)){
        if(count>word(body,0x7c)){word(body,0x7c)=count;o.native_site=0xc37d74;void* grown=c.physics_allocate_00bf55be(count*4,m);
            U i=0;void* output=grown;while(i<word(body,0x78)){if(output)word(output)=word(at(ptr(body,0x74),i*4));++i;output=at(output,4);}
            if(void* old=ptr(body,0x74))release69(old,0xc37da6,m,c,o);ptr(body,0x74,grown);
        }
        U i=word(body,0x78);while(i<count){void* output=at(ptr(body,0x74),i*4);if(output)word(output)=0;++i;}
    }
    word(body,0x78)=count;
}
void clear_native_dyn_body_manifolds_00c43aa0(void* body,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){
    while(word(body,0x78)){
        void* manifold=ptr(ptr(body,0x74));void* pool=ptr(ptr(ptr(body),0x444),0xb0);o.captured=manifold;
        o.native_site=0xc43acc;remove_native_dyn_manifold_reference_00c37d30(ptr(manifold,0xcc),manifold,m,c,o);
        o.native_site=0xc43ad9;remove_native_dyn_manifold_reference_00c37d30(ptr(manifold,0xd0),manifold,m,c,o);
        ptr(ptr(manifold,0xdc),0xd8,ptr(manifold,0xd8));ptr(ptr(manifold,0xd8),0xdc,ptr(manifold,0xdc));
        ptr(manifold,0xdc,ptr(pool,0xc));word(pool,0x1d0)=word(pool,0x1d0)-1;ptr(pool,0xc,manifold);
    }
}
void destroy_native_dyn_body_storage_00c43c00(void* body,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){
    o.native_site=0xc43c24;clear_native_dyn_body_manifolds_00c43aa0(body,m,c,o);
    if(!(word(body,0x50)&1)){
        void* motion=ptr(body,4);void* world=ptr(body);ptr(ptr(motion,0xc4),0xc0,ptr(motion,0xc0));
        void* next=ptr(motion,0xc0);void* previous=ptr(motion,0xc4);void* pool=at(world,0x294);ptr(next,0xc4,previous);
        ptr(motion,0xc4,ptr(pool,0xc));word(pool,0x1a0)=word(pool,0x1a0)-1;ptr(pool,0xc,motion);
    }
    if(void* data=ptr(body,0x74))release69(data,0xc43c78,m,c,o);
}
void clear_native_dyn_world_bodies_00c4daa0(void* world,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){
    o.world=world;
    active_bodies(world,0xe0,0xe4,0xc4dad1,0xc4daee,0xc4dafa,m,c,o);
    active_bodies(world,0x204,0x208,0xc4db3c,0xc4db5d,0xc4db69,m,c,o);
    recycle_bodies(world,0xe0,0xe4,0x16c,0x58,0x164,0x5c,0xc4dba1,0xc4dbb5,m,c,o);
    recycle_bodies(world,0x204,0x208,0x290,0x17c,0x288,0x180,0xc4dc01,0xc4dc15,m,c,o);
    if(word(world,0x43c))word(world,0x43c)=0;
}
void destroy_native_dyn_manifold_pool_00406f20(void* pool,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){
    o.native_site=0x406f2b;c.physics_delete_section(at(pool,0x1d8));pages(pool,0,0x406f3e,0x406f55,m,c,o);
}
void destroy_native_dyn_bucket_storage_00407210(void* p,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){
    if(!word(p,4))return;U i=0,offset=0;
    while(i<word(p,8)){if(void* data=ptr(at(ptr(p,4),offset)))release69(data,0x40722d,m,c,o);++i;offset+=12;}
    release69(ptr(p,4),0x407245,m,c,o);
}
void destroy_native_dyn_scene_00c32250(void* scene,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){
    if(void* manager=ptr(scene,0xac)){o.native_site=0xc32284;o.captured=manager;c.physics_delete_broadphase_004043d0(manager,1,m,o.sap);word(scene,0xac)=0;}
    if(void* pool=ptr(scene,0xb0)){o.native_site=0xc3229c;destroy_native_dyn_manifold_pool_00406f20(pool,m,c,o);release65(pool,0xc322a2,m,c,o);word(scene,0xb0)=0;}
    if(void* p=ptr(scene,0xd8))release69(p,0xc322bf,m,c,o);
    if(void* p=ptr(scene,0xcc))release69(p,0xc322d2,m,c,o);
    if(void* p=ptr(scene,0xc0))release69(p,0xc322e5,m,c,o);
    if(void* p=ptr(scene,0xb4))release69(p,0xc322f8,m,c,o);
}
void destroy_native_dyn_world_00c4dc60(void* world,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){
    o.world=world;o.native_site=0xc4dc83;clear_native_dyn_world_bodies_00c4daa0(world,m,c,o);
    if(void* scene=ptr(world,0x444)){o.native_site=0xc4dc93;destroy_native_dyn_scene_00c32250(scene,m,c,o);release65(scene,0xc4dc99,m,c,o);word(world,0x444)=0;}
    if(void* p=ptr(world,0x480))release69(p,0xc4dcb6,m,c,o);
    if(void* p=ptr(world,0x474))release69(p,0xc4dcc9,m,c,o);
    if(void* p=ptr(world,0x468))release69(p,0xc4dcdc,m,c,o);
    if(void* p=ptr(world,0x45c))release69(p,0xc4dcef,m,c,o);
    o.native_site=0xc4dcfd;destroy_native_dyn_bucket_storage_00407210(at(world,0x448),m,c,o);
    if(void* p=ptr(world,0x438))release69(p,0xc4dd0d,m,c,o);
    pages(world,0x294,0xc4dd2a,0xc4dd48,m,c,o);pages(world,0x170,0xc4dd6a,0xc4dd88,m,c,o);pages(world,0x4c,0xc4dd9e,0xc4ddb6,m,c,o);
}
void delete_native_dyn_profile_node_00c35400(void* node,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){
    U cursor=word(node,4);while(cursor!=word(node,4)+word(node,8)*4){o.native_site=0xc35415;delete_native_dyn_profile_node_00c35400(ptr(pointer(cursor)),m,c,o);cursor+=4;}
    if(word(node,0x28)>=16){o.native_site=0xc35434;o.captured=ptr(node,0x14);c.physics_free_profile_text_00bf65ac(o.captured);}
    word(node,0x28)=15;word(node,0x24)=0;*static_cast<volatile unsigned char*>(at(node,0x14))=0;
    if(void* p=ptr(node,4))release69(p,0xc35453,m,c,o);release65(node,0xc3545c,m,c,o);
}
void destroy_native_dyn_engine_00c421b0(void* engine,const AvoidZoneDynHullMemory& m,NativeGamePhysicsLifetimeCalls& c,NativeGamePhysicsLifetimeProgress& o){
    if(void* profile=ptr(engine,0xc)){o.native_site=0xc421dc;delete_native_dyn_profile_node_00c35400(ptr(profile),m,c,o);release65(profile,0xc421e2,m,c,o);}
    if(void* tasks=ptr(engine,0x10)){o.native_site=0xc421f2;o.captured=tasks;c.physics_destroy_tasks_00c40ff0(tasks,m);release65(tasks,0xc421f8,m,c,o);}
    if(void* p=ptr(engine))release69(p,0xc42207,m,c,o);
}
NativeGamePhysicsLifetimeOperation::~NativeGamePhysicsLifetimeOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void NativeGamePhysicsLifetimeOperation::acknowledge_diagnostic_cleanup() noexcept{if(phase==Phase::failed)phase=Phase::diagnostic_retired;}
void destroy_native_game_physics_00c4dde0(void* header,NativeGamePhysicsLifetimeContext& x,NativeGamePhysicsLifetimeOperation& o){
    if(o.phase!=NativeGamePhysicsLifetimeOperation::Phase::fresh)throw std::logic_error("native physics destruction cannot be replayed");
    o.context=&x;o.header=header;o.phase=NativeGamePhysicsLifetimeOperation::Phase::running;
    try{auto& c=x.calls;const auto& world_memory=x.dynamics.world.scene->memory;const auto& engine_memory=x.dynamics.engine.memory;
        U cursor=word(header);while(cursor!=word(header)+word(header,4)*4){o.cursor=pointer(cursor);
            if(void* world=ptr(pointer(cursor))){o.native_site=0xc4ddf8;destroy_native_dyn_world_00c4dc60(world,world_memory,c,o);release65(world,0xc4ddfe,world_memory,c,o);}cursor+=4;
        }
        if(void* engine=*x.dynamics.engine.engine_slot_0109e9fc){o.native_site=0xc4de22;destroy_native_dyn_engine_00c421b0(engine,engine_memory,c,o);release65(engine,0xc4de28,engine_memory,c,o);*x.dynamics.engine.engine_slot_0109e9fc=nullptr;}
        o.phase=NativeGamePhysicsLifetimeOperation::Phase::complete;
    }catch(...){o.phase=NativeGamePhysicsLifetimeOperation::Phase::failed;throw;}
}
} // namespace bsp
