#include "bsp/native_game_lifetime.hpp"
#include "bsp/native_game_construction.hpp"
#include "bsp/native_lua_objects.hpp"
#include "bsp/native_input_configuration_owner.hpp"
#include "bsp/native_input_settings_tree_cleanup.hpp"
#include "bsp/native_resource_manager_lifetime.hpp"
#include <Windows.h>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
static_assert(sizeof(void*)==4);
void* at(void* p,U n) noexcept {return static_cast<char*>(p)+n;}
U word(void* p,U n) noexcept {U v;std::memcpy(&v,at(p,n),4);return v;}
void word(void* p,U n,U v) noexcept {std::memcpy(at(p,n),&v,4);}
void* pointer(void* p,U n) noexcept {return reinterpret_cast<void*>(word(p,n));}
NativeResourceManagerContext& resource_context(NativeGameLifetimeCalls& calls,
    NativeResourceManagerContext* resources,NativeGameProfileLifetimeContext* profile){
    if(!resources||!profile||&profile->calls!=&calls)
        throw std::invalid_argument("native game resource cleanup requires its actual contexts");
    auto& r=resources->strings;auto& p=profile->strings;
    if(&resources->actual_lifetime_manager_01090aa0!=&p.actual_manager_publication_01090aa0||
       &r.actual_manager_publication_01090aa0!=&p.actual_manager_publication_01090aa0||
       &r.actual_published_01090aa8!=&p.actual_published_01090aa8||
       &r.actual_small_returns_disabled_01090aa4!=&p.actual_small_returns_disabled_01090aa4)
        throw std::invalid_argument("native game resources must share its actual string and lifetime publications");
    return *resources;
}
void terminal(void* p,U dec_site,U terminal_site,NativeGameLifetimeCalls& c,
    NativeGameLifetimeProgress& o) {
    o.native_site=dec_site;
    if(c.interlocked_decrement(static_cast<volatile std::int32_t*>(at(p,4)))==0) {
        o.native_site=terminal_site;c.virtual_terminal(p);
    }
}
void normal(NativeGameStorage& game,NativeGameLifetimeContext& x,NativeGameLifetimeOperation& o) {
    void* g=&game;auto& c=x.calls;word(g,0,0x00ce7cb8);o.unwind_state=0x24;
    x.small_returns_disabled_01090aa4=1;
    o.native_site=0x4dcfcb;destroy_native_game_nested_storage_004d27c0(at(g,0x30),c,o);
    o.native_site=0x4dcfd3;c.call_00c4dde0(pointer(g,0x14),x.physics,o.physics);
    o.native_site=0x4dcfde;c.call_0076a760(at(g,0x1ef0),x.embedded);
    o.native_site=0x4dcfe3;c.call_008d88f0();
    o.native_site=0x4dcfee;c.call_004bf930(at(g,0x5f0),o.containers);
    const auto scalar=[&](void* volatile& cell,U slot,U site) {
        void* p=cell;if(p){o.native_site=site;c.virtual_scalar(p,slot,1);cell=nullptr;}
    };
    scalar(x.publication_00e18678,0,0x4dd005);scalar(x.publication_00e1867c,0,0x4dd01d);
    o.native_site=0x4dd026;c.call_004a9ac0(x.classes,x.profile,o.classes);
    if(void* p=x.publication_00e19900) {
        o.native_site=0x4dd037;c.call_006b9380(p,x.profile,o.awards);
        o.native_site=0x4dd03d;c.free_00bf65ac(p);x.publication_00e19900=nullptr;
    }
    scalar(x.publication_00e18db0,0xc,0x4dd05c);scalar(x.publication_00e19698,0xc,0x4dd075);
    scalar(x.publication_00e1930c,0xc,0x4dd08e);scalar(x.publication_00e198bc,0,0x4dd0a6);
    if(void* p=pointer(g,0x19c8)) {o.native_site=0x4dd0bf;c.virtual_scalar(p,0xc,1);word(g,0x19c8,0);}
    scalar(x.movie_00e18d48,0xc,0x4dd0d8);
    o.native_site=0x4dd0e1;c.call_004c0c30(x.singletons,o.singletons[0]);o.native_site=0x4dd0e6;c.call_004c0ce0(x.singletons,o.singletons[1]);
    o.native_site=0x4dd0eb;c.call_00b6cf90(x.lua_globals,x.profile,o.lua_globals);o.native_site=0x4dd0f0;c.call_004c0d90(x.singletons,o.singletons[2]);
    if(void* p=pointer(g,0x21f4))terminal(p,0x4dd109,0x4dd115,c,o); // field remains untouched
    o.native_site=0x4dd11e;c.free_00bf65ac(pointer(g,0x2200));
    o.native_site=0x4dd126;void* manager=c.call_004c1400(x.resources,x.profile);
    o.native_site=0x4dd12d;c.call_00b806f0(manager,x.resources,x.profile);
    o.native_site=0x4dd134;c.call_004c7dd0(g,o.containers);
    // Native captures the first grid BEFORE clearing the game publication.
    void* first_grid=x.grid_00e19b0c;x.game_00e188a8=nullptr;
    if(first_grid){o.native_site=0x4dd14f;c.virtual_scalar(first_grid,0,1);x.grid_00e19b0c=nullptr;}
    scalar(x.grid_00e19b08,0,0x4dd167);scalar(x.grid_00e19b04,0,0x4dd17f);
    o.native_site=0x4dd18d;c.call_0041cc80(at(g,0x1ee8));o.unwind_state=0x23;
    if(void* begin=pointer(g,0x7190)) {
        void* end=pointer(g,0x7194);o.native_site=0x4dd1b0;
        release_native_game_pointer_range_004cc760(begin,end,c,o);
        o.native_site=0x4dd1b9;c.free_00bf65ac(pointer(g,0x7190));
    }
    word(g,0x7190,0);word(g,0x7194,0);word(g,0x7198,0);o.unwind_state=0x22;
    o.native_site=0x4dd1d8;c.call_004cb220(at(g,0x7178),0,o.containers);
    o.native_site=0x4dd1e0;c.free_00bf6989(pointer(g,0x7178));
    o.native_site=0x4dd1ee;c.call_004c4b40(at(g,0x716c),o.containers);o.unwind_state=0x20;
    const auto string=[&](U offset,U getter,U release) {
        if(void* p=pointer(g,offset+4)) {
            U size=word(g,offset)+1;o.native_site=getter;void* pool=c.call_00419cc0(p,size,1,x.profile);
            o.native_site=release;c.call_00bd1510(pool,p,size,1,x.profile);
        }
    };
    string(0x7164,0x4dd20f,0x4dd216);o.unwind_state=0x1f;
    o.native_site=0x4dd230;c.array_destroy_00bf7c6e(at(g,0x7134),0xc,4,0x4c4600,x.arrays,o.arrays[0]);
    o.unwind_state=0x1e;string(0x2198,0x4dd251,0x4dd258);o.unwind_state=0x1d;
    o.native_site=0x4dd268;c.call_0076f000(at(g,0x1ef0),x.embedded,o.embedded);o.unwind_state=0x1c;
    o.native_site=0x4dd278;c.call_00b669a0(at(g,0x1a0c));
    constexpr U offsets[]={0x19e4,0x19e0,0x19dc,0x19d8,0x19d4};
    constexpr U decs[]={0x4dd290,0x4dd2b7,0x4dd2de,0x4dd305,0x4dd32c};
    constexpr U terms[]={0x4dd29c,0x4dd2c3,0x4dd2ea,0x4dd311,0x4dd338};
    for(U i=0;i<5;++i) {void* p=pointer(g,offsets[i]);o.unwind_state=0x1b-static_cast<int>(i);
        if(p){terminal(p,decs[i],terms[i],c,o);word(g,offsets[i],0);}}
    constexpr U list_sites[]={0x4dd346,0x4dd351,0x4dd35c,0x4dd367,0x4dd372,0x4dd37d,0x4dd388,0x4dd393};
    for(U i=0;i<8;++i){o.native_site=list_sites[i];c.call_004bf8e0(at(g,0x19b8-i*0xc),o.containers);}
    o.unwind_state=0xe;o.native_site=0x4dd3a3;c.call_007ff9f0(at(g,0x1944),x.profile);
    using Tree=void(NativeGameLifetimeCalls::*)(void*,void*,void*,void*,void*,void*,NativeGameProfileLifetimeContext*,NativeGameTreeLifetimeProgress&);
    U iterator[2]; // native private8h output, intentionally not initialized
    const auto tree=[&](U offset,U state,U site,U free_site,Tree f) {
        void* header=at(g,offset);void* head=pointer(header,4);void* first=pointer(head,0);
        o.unwind_state=static_cast<int>(state);o.native_site=site;
        (c.*f)(header,iterator,header,first,header,head,x.profile,o.trees);
        o.native_site=free_site;c.free_00bf65ac(pointer(header,4));word(header,4,0);word(header,8,0);
    };
    tree(0x1930,0xd,0x4dd3c6,0x4dd3cf,&NativeGameLifetimeCalls::call_004d1a50);
    o.unwind_state=0xc;o.native_site=0x4dd3f5;c.array_destroy_00bf7c6e(at(g,0x1008),0x118,8,0x4cb2f0,x.arrays,o.arrays[1]);
    o.unwind_state=0xb;o.native_site=0x4dd412;c.array_destroy_00bf7c6e(at(g,0x748),0x118,8,0x4cb2f0,x.arrays,o.arrays[2]);
    o.unwind_state=0xa;o.native_site=0x4dd422;c.call_007fd8a0(at(g,0x650),x.profile,o.profile);
    o.native_site=0x4dd42f;c.call_004cf3f0(at(g,0x638),o.containers);
    o.native_site=0x4dd438;c.free_00bf65ac(pointer(g,0x63c));word(g,0x63c,0);
    tree(0x628,8,0x4dd461,0x4dd46a,&NativeGameLifetimeCalls::call_004d22f0);
    o.unwind_state=7;string(0x600,0x4dd495,0x4dd49c);
    o.native_site=0x4dd4a7;c.call_004c2ce0(at(g,0x5f0),o.containers);
    o.native_site=0x4dd4b2;c.call_004c4a50(at(g,0x5d8),o.containers);
    tree(0x5c8,4,0x4dd4d5,0x4dd4de,&NativeGameLifetimeCalls::call_004d2000);
    tree(0x5bc,3,0x4dd50a,0x4dd513,&NativeGameLifetimeCalls::call_004d41a0);
    tree(0x5b0,2,0x4dd53f,0x4dd548,&NativeGameLifetimeCalls::call_004cef40);
    o.unwind_state=1;o.native_site=0x4dd56b;c.array_destroy_00bf7c6e(at(g,0x560),0x10,5,0x4cafd0,x.arrays,o.arrays[3]);
    o.unwind_state=0;o.native_site=0x4dd577;c.call_004dceb0(at(g,0x3c));
    if(void* p=pointer(g,0x24)){o.native_site=0x4dd585;c.free_00bf65ac(p);}
    word(g,0x24,0);word(g,0x28,0);word(g,0x2c,0);
}
void begin(NativeGameStorage& g,NativeGameLifetimeContext& x,NativeGameLifetimeOperation& o) {
    if(o.phase!=NativeGameLifetimeOperation::Phase::fresh)throw std::logic_error("native game destruction cannot be replayed");
    o.owner=&g;o.context=&x;o.phase=NativeGameLifetimeOperation::Phase::running;
}
}
std::int32_t NativeGameLifetimeCalls::interlocked_decrement(volatile std::int32_t* p){return ::InterlockedDecrement(reinterpret_cast<volatile LONG*>(p));}
void NativeGameLifetimeCalls::free_00bf65ac(void* p){::operator delete(p);}
void NativeGameLifetimeCalls::free_00bf6989(void* p){std::free(p);}
NativeGameLifetimeOperation::~NativeGameLifetimeOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void NativeGameLifetimeOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::failed){
        for(const auto& array:arrays)if(array.phase==NativeGameArrayLifetimeOperation::Phase::running||array.phase==NativeGameArrayLifetimeOperation::Phase::failed)std::terminate();
        if(embedded.phase==NativeGameEmbeddedLifetimeOperation::Phase::running||embedded.phase==NativeGameEmbeddedLifetimeOperation::Phase::failed)std::terminate();
        if(profile.phase==NativeGameProfileLifetimeOperation::Phase::running||profile.phase==NativeGameProfileLifetimeOperation::Phase::failed)std::terminate();
        if(awards.phase==NativeAwardRegistryLifetimeOperation::Phase::running||awards.phase==NativeAwardRegistryLifetimeOperation::Phase::failed)std::terminate();
        for(const auto& singleton:singletons)if(singleton.phase==NativeGameSingletonLifetimeOperation::Phase::running||singleton.phase==NativeGameSingletonLifetimeOperation::Phase::failed)std::terminate();
        if(lua_globals.phase==NativeGameLuaGlobalsLifetimeOperation::Phase::running||lua_globals.phase==NativeGameLuaGlobalsLifetimeOperation::Phase::failed)std::terminate();
        if(classes.phase==NativeGameClassCleanupOperation::Phase::running||classes.phase==NativeGameClassCleanupOperation::Phase::failed)std::terminate();
        if(physics.phase==NativeGamePhysicsLifetimeOperation::Phase::running||physics.phase==NativeGamePhysicsLifetimeOperation::Phase::failed)std::terminate();
        phase=Phase::diagnostic_retired;
    }
}
void NativeGameLifetimeCalls::call_008d88f0(){native_game_cleanup_noop_008d88f0();}
void NativeGameLifetimeCalls::call_00c4dde0(void* header,NativeGamePhysicsLifetimeContext* x,NativeGamePhysicsLifetimeOperation& o){
    if(!x||&x->calls!=this||!x->dynamics.world.scene)
        throw std::invalid_argument("native game physics cleanup requires its construction contexts");
    const auto& engine=x->dynamics.engine;const auto& scene=*x->dynamics.world.scene;
    if(!engine.engine_slot_0109e9fc||scene.engine_slot!=engine.engine_slot_0109e9fc||
       !engine.memory.allocate||!engine.memory.release||!scene.memory.allocate||!scene.memory.release)
        throw std::invalid_argument("native physics cleanup requires matching engine publication and actual allocators");
    destroy_native_game_physics_00c4dde0(header,*x,o);
}
void NativeGameLifetimeCalls::call_004a9ac0(NativeGameClassCleanupContext* c,NativeGameProfileLifetimeContext* p,NativeGameClassCleanupOperation& o){
    if(!c||!p||&c->calls!=this||&p->calls!=this||!c->actual_vectors_00e1875c)
        throw std::invalid_argument("native game class cleanup requires its actual contexts");
    auto& a=c->strings;auto& b=p->strings;
    if(&a.actual_manager_publication_01090aa0!=&b.actual_manager_publication_01090aa0||
       &a.actual_published_01090aa8!=&b.actual_published_01090aa8||
       &a.actual_small_returns_disabled_01090aa4!=&b.actual_small_returns_disabled_01090aa4)
        throw std::invalid_argument("native game class cleanup must share its string and lifetime publications");
    clear_native_game_class_globals_004a9ac0(*c,o);
}
void* NativeGameLifetimeCalls::call_004c1400(NativeResourceManagerContext* r,NativeGameProfileLifetimeContext* p){
    return get_native_resource_manager_004c1400(resource_context(*this,r,p));
}
void NativeGameLifetimeCalls::call_00b806f0(void* manager,NativeResourceManagerContext* r,NativeGameProfileLifetimeContext* p){
    clear_native_resource_manager_cache_00b806f0(manager,resource_context(*this,r,p));
}
void NativeGameLifetimeCalls::call_00b6cf90(NativeGameLuaGlobalsLifetimeContext* c,NativeGameProfileLifetimeContext* p,NativeGameLuaGlobalsLifetimeOperation& o){
    if(!c||&c->calls!=this||!p||&p->calls!=this)throw std::invalid_argument("native Lua globals cleanup requires the game's actual contexts");
    clear_native_game_lua_globals_00b6cf90(*c,*p,o);
}
void NativeGameLifetimeCalls::call_004c0c30(NativeGameSingletonLifetimeContext* c,NativeGameSingletonLifetimeOperation& o){
    if(!c||&c->calls!=this)throw std::invalid_argument("native singleton deletion requires the game's actual context");
    delete_native_game_singleton_004c0c30(*c,o);
}
void NativeGameLifetimeCalls::call_004c0ce0(NativeGameSingletonLifetimeContext* c,NativeGameSingletonLifetimeOperation& o){
    if(!c||&c->calls!=this)throw std::invalid_argument("native singleton deletion requires the game's actual context");
    delete_native_game_singleton_004c0ce0(*c,o);
}
void NativeGameLifetimeCalls::call_004c0d90(NativeGameSingletonLifetimeContext* c,NativeGameSingletonLifetimeOperation& o){
    if(!c||&c->calls!=this)throw std::invalid_argument("native singleton deletion requires the game's actual context");
    delete_native_game_singleton_004c0d90(*c,o);
}
void NativeGameLifetimeCalls::array_destroy_00bf7c6e(void* p,U stride,U count,U destructor,
    NativeGameArrayLifetimeContext* context,NativeGameArrayLifetimeOperation& operation){
    if(!context||&context->calls!=this)throw std::invalid_argument("native game array destruction requires its actual lifetime services");
    destroy_native_game_array_00bf7c6e(p,stride,count,destructor,*context,operation);
}
void NativeGameLifetimeCalls::call_0076a760(void* p,NativeGameEmbeddedLifetimeContext* context){
    if(!context||&context->calls!=this)throw std::invalid_argument("native embedded game cleanup requires its actual context");
    disable_native_game_embedded_owners_0076a760(p,context->flag_00e0af14);
}
void NativeGameLifetimeCalls::call_0076f000(void* p,NativeGameEmbeddedLifetimeContext* context,NativeGameEmbeddedLifetimeOperation& operation){
    if(!context||&context->calls!=this)throw std::invalid_argument("native embedded game cleanup requires its actual context");
    destroy_native_game_embedded_state_0076f000(p,*context,operation);
}
void NativeGameLifetimeCalls::call_0041cc80(void* p){NativeGameEmbeddedLifetimeCalls::call_0041cc80(p);}
void NativeGameLifetimeCalls::call_00b669a0(void* p){close_native_lua_state_00b669a0(*static_cast<NativeLuaStateStorage*>(p));}
void NativeGameLifetimeCalls::call_004dceb0(void* p){destroy_native_input_configuration_004dceb0(p);}
void* NativeGameLifetimeCalls::allocate_00bf55be(U n){return NativeGameContainerLifetimeCalls::allocate_00bf55be(n);}
void NativeGameLifetimeCalls::call_004d1a50(void* p,void* out,void* fo,void* first,void* lo,void* last,NativeGameProfileLifetimeContext* c,NativeGameTreeLifetimeProgress& o){
    if(!c||&c->calls!=this)throw std::invalid_argument("native game tree cleanup requires its actual string context");
    o.owner=p;o.native_site=0x004d1a50;profile_call_004d1a50(p,out,fo,first,lo,last,*c);
}
void NativeGameLifetimeCalls::call_004cef40(void* p,void* out,void* fo,void* first,void* lo,void* last,NativeGameProfileLifetimeContext* c,NativeGameTreeLifetimeProgress& o){
    if(!c||&c->calls!=this)throw std::invalid_argument("native game tree cleanup requires its actual string context");
    o.owner=p;o.native_site=0x004cef40;clear_native_input_int_only_full_range_004cef40(p,out,fo,first,lo,last,c->actual_strings);
}
void NativeGameLifetimeCalls::call_004d2000(void* p,void* out,void* fo,void* first,void* lo,void* last,NativeGameProfileLifetimeContext* c,NativeGameTreeLifetimeProgress& o){
    if(!c||&c->calls!=this)throw std::invalid_argument("native game tree cleanup requires its actual string context");
    clear_native_game_tree_full_range_004d2000(p,out,fo,first,lo,last,*c,o);
}
void NativeGameLifetimeCalls::call_004d22f0(void* p,void* out,void* fo,void* first,void* lo,void* last,NativeGameProfileLifetimeContext* c,NativeGameTreeLifetimeProgress& o){
    if(!c||&c->calls!=this)throw std::invalid_argument("native game tree cleanup requires its actual string context");
    clear_native_game_tree_full_range_004d22f0(p,out,fo,first,lo,last,*c,o);
}
void NativeGameLifetimeCalls::call_004d41a0(void* p,void* out,void* fo,void* first,void* lo,void* last,NativeGameProfileLifetimeContext* c,NativeGameTreeLifetimeProgress& o){
    if(!c||&c->calls!=this)throw std::invalid_argument("native game tree cleanup requires its actual string context");
    clear_native_game_tree_full_range_004d41a0(p,out,fo,first,lo,last,*c,o);
}
void NativeGameLifetimeCalls::call_004c7dd0(void* p,NativeGameContainerLifetimeProgress& o){clear_native_game_payload_lists_004c7dd0(p,*this,o);}
void NativeGameLifetimeCalls::call_006b9380(void* p,NativeGameProfileLifetimeContext* c,NativeAwardRegistryLifetimeOperation& o){
    if(!c||&c->calls!=this)throw std::invalid_argument("award registry destruction requires the game's actual string context");
    destroy_native_award_registry_006b9380(p,*c,o);
}
void NativeGameLifetimeCalls::call_004bf930(void* p,NativeGameContainerLifetimeProgress& o){destroy_native_game_scene_records_004bf930(p,*this,o);}
void NativeGameLifetimeCalls::call_004bf8e0(void* p,NativeGameContainerLifetimeProgress& o){clear_native_game_unit_links_004bf8e0(p,*this,o);}
void NativeGameLifetimeCalls::call_004c2ce0(void* p,NativeGameContainerLifetimeProgress& o){clear_native_game_scene_links_004c2ce0(p,*this,o);}
void NativeGameLifetimeCalls::call_004cf3f0(void* p,NativeGameContainerLifetimeProgress& o){clear_native_game_small_string_nodes_004cf3f0(p,*this,o);}
void NativeGameLifetimeCalls::call_004c4b40(void* p,NativeGameContainerLifetimeProgress& o){destroy_native_game_pointer_list_004c4b40(p,*this,o);}
void NativeGameLifetimeCalls::call_004c4a50(void* p,NativeGameContainerLifetimeProgress& o){destroy_native_game_pointer_blocks_004c4a50(p,*this,o);}
void NativeGameLifetimeCalls::call_004cb220(void* p,U requested,NativeGameContainerLifetimeProgress& o){resize_native_game_reference_slots_004cb220(p,requested,*this,o);}
void* NativeGameLifetimeCalls::call_00419cc0(void*,U,U,NativeGameProfileLifetimeContext* context){
    if(!context||&context->calls!=this)throw std::invalid_argument("native game string cleanup requires its actual profile context");
    return profile_pool_00419cc0(context->strings);
}
void NativeGameLifetimeCalls::call_00bd1510(void* pool,void* p,U size,U,NativeGameProfileLifetimeContext* context){
    if(!context||&context->calls!=this)throw std::invalid_argument("native game string cleanup requires its actual profile context");
    profile_return_00bd1510(static_cast<NativeStringPoolStorage*>(pool),p,size,context->strings);
}
void NativeGameLifetimeCalls::call_007ff9f0(void* p,NativeGameProfileLifetimeContext* context){
    if(!context||&context->calls!=this)throw std::invalid_argument("native race cleanup requires its actual profile context");
    NativeGameProfileLifetimeProgress progress;destroy_native_game_race_record_007ff9f0(p,*context,progress);
}
void NativeGameLifetimeCalls::call_007fd8a0(void* p,NativeGameProfileLifetimeContext* context,NativeGameProfileLifetimeOperation& operation){
    if(!context||&context->calls!=this)throw std::invalid_argument("native profile cleanup requires its actual context");
    destroy_native_game_profile_007fd8a0(p,*context,operation);
}
void destroy_native_game_nested_storage_004d27c0(void* cell,NativeGameLifetimeCalls& c,NativeGameLifetimeProgress& o) {
    if(void* p=pointer(cell,0)) {
        if(void* q=pointer(p,0x14)){o.native_site=0x4d27d5;c.free_00bf65ac(q);}
        word(p,0x14,0);word(p,0x18,0);word(p,0x1c,0);
        if(void* q=pointer(p,4)){o.native_site=0x4d27ee;c.free_00bf65ac(q);}
        word(p,4,0);word(p,8,0);word(p,0xc,0);
        o.native_site=0x4d2800;c.free_00bf65ac(p);word(cell,0,0);
    }
}
void release_native_game_pointer_range_004cc760(void* first,void* last,NativeGameLifetimeCalls& c,NativeGameLifetimeProgress& o) {
    for(void* cursor=first;cursor!=last;cursor=at(cursor,4)) {
        if(void* p=pointer(cursor,0)){terminal(p,0x4cc77c,0x4cc788,c,o);word(cursor,0,0);}
    }
}
void destroy_native_game_004dcf90(NativeGameStorage& g,NativeGameLifetimeContext& x,NativeGameLifetimeOperation& o) {
    begin(g,x,o);try{normal(g,x,o);o.phase=NativeGameLifetimeOperation::Phase::complete;}
    catch(...){o.phase=NativeGameLifetimeOperation::Phase::failed;throw;}
}
NativeGameStorage* delete_native_game_004de270(NativeGameStorage& g,U flags,NativeGameLifetimeContext& x,NativeGameLifetimeOperation& o) {
    begin(g,x,o);auto* result=&g;
    try{normal(g,x,o);if(flags&1){o.native_site=0x4de280;x.calls.free_00bf6989(result);}o.phase=NativeGameLifetimeOperation::Phase::complete;return result;}
    catch(...){o.phase=NativeGameLifetimeOperation::Phase::failed;throw;}
}
} // namespace bsp
