#include "bsp/native_game_profile_lifetime.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_render_resource_record.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_vfs_string_tree.hpp"
#include "bsp/native_vfs_sequence_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;using Progress=NativeGameProfileLifetimeProgress;
static_assert(sizeof(void*)==4);
void* at(void* p,U n) noexcept{return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
U word(void* p,U n) noexcept{U v;std::memcpy(&v,at(p,n),4);return v;}
void word(void* p,U n,U v) noexcept{std::memcpy(at(p,n),&v,4);}
void* pointer(void* p,U n=0) noexcept{return reinterpret_cast<void*>(word(p,n));}
void require_full(void* tree,void* first_owner,void* first,void* last_owner,void* last){
    if(first_owner!=tree||last_owner!=tree||first!=pointer(pointer(tree,4))||last!=pointer(tree,4))
        throw std::invalid_argument("profile library adapter requires the actual full range");
}
void* finish_full(void* tree,void* output){
    void* head=pointer(tree,4);word(head,4,reinterpret_cast<U>(head));
    head=pointer(tree,4);word(tree,8,0);word(head,0,reinterpret_cast<U>(head));
    head=pointer(tree,4);word(head,8,reinterpret_cast<U>(head));
    void* first=pointer(pointer(tree,4));word(output,0,reinterpret_cast<U>(tree));word(output,4,reinterpret_cast<U>(first));return output;
}
void release(void* p,U offset,U getter,U returned,NativeGameProfileLifetimeContext& c,Progress& state){
    if(void* data=pointer(p,offset+4)){
        const U size=word(p,offset)+1;state.native_site=getter;auto* pool=c.calls.profile_pool_00419cc0(c.strings);
        state.native_site=returned;c.calls.profile_return_00bd1510(pool,data,size,c.strings);
    }
}
}
NativeStringPoolStorage* NativeGameProfileLifetimeCalls::profile_pool_00419cc0(NativeStringRawPoolContext& c){return native_string_pool_get_or_create_00419cc0(c.actual_published_01090aa8,c.actual_manager_publication_01090aa0);}
void NativeGameProfileLifetimeCalls::profile_return_00bd1510(NativeStringPoolStorage* pool,void* p,U size,NativeStringRawPoolContext& c){return_native_string_pool_00bd1510(pool,p,size,c.actual_small_returns_disabled_01090aa4);}
void NativeGameProfileLifetimeCalls::profile_invalid_parameter_00bf6713(){_invalid_parameter_noinfo();}
void NativeGameProfileLifetimeCalls::profile_call_007fd780(void* p,NativeGameProfileLifetimeContext& c,NativeMissionProgressOperation& child){destroy_native_mission_progress_007fd780(p,c.actual_strings,*this,child);}
void NativeGameProfileLifetimeCalls::profile_call_004d05e0(void* p,NativeGameProfileLifetimeContext& c){clear_native_render_resource_aliases_004d05e0(p,c.strings);}
void NativeGameProfileLifetimeCalls::profile_call_00432050(void* first,void* last,NativeGameProfileLifetimeContext& c){destroy_native_string_header_range_00432050(first,last,c.strings);}
void* NativeGameProfileLifetimeCalls::profile_call_0058d860(void* tree,void* output,void* first_owner,void* first,void* last_owner,void* last,NativeGameProfileLifetimeContext& c){
    require_full(tree,first_owner,first,last_owner,last);return clear_native_profile_counter_full_range_0058d860(tree,output,c.actual_strings,*this);
}
void* NativeGameProfileLifetimeCalls::profile_call_007fb230(void* tree,void* output,void* first_owner,void* first,void* last_owner,void* last,NativeGameProfileLifetimeContext& c){
    require_full(tree,first_owner,first,last_owner,last);erase_native_profile_transient_subtree_007fa880(tree,pointer(pointer(tree,4),4),c.actual_strings,*this);return finish_full(tree,output);
}
void* NativeGameProfileLifetimeCalls::profile_call_004d1a50(void* tree,void* output,void* first_owner,void* first,void* last_owner,void* last,NativeGameProfileLifetimeContext& c){
    SingletonLifetimeCallbacks callbacks{this,nullptr,[](void* p){static_cast<NativeGameProfileLifetimeCalls*>(p)->profile_invalid_parameter_00bf6713();}};
    return erase_native_vfs_string_range_004d1a50(tree,output,first_owner,first,last_owner,last,c.actual_strings,callbacks);
}
void NativeGameProfileLifetimeCalls::profile_call_007f8310(void* p){destroy_native_vfs_plain_list_007f8310(p,nullptr);}
NativeGameProfileLifetimeOperation::~NativeGameProfileLifetimeOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void NativeGameProfileLifetimeOperation::acknowledge_diagnostic_cleanup() noexcept{
    using Child=NativeMissionProgressOperation;if(phase==Phase::failed){if(mission.phase==Child::Phase::running||mission.phase==Child::Phase::failed)std::terminate();phase=Phase::diagnostic_retired;}
}
void destroy_native_profile_three_strings_0061f630(void* p,NativeGameProfileLifetimeContext& c,Progress& state){
    state.element_unwind_state=1;release(p,0x10,0x0061f666,0x0061f66d,c,state);
    state.element_unwind_state=0;release(p,8,0x0061f688,0x0061f68f,c,state);
    state.element_unwind_state=-1;release(p,0,0x0061f6ac,0x0061f6b3,c,state);
}
void destroy_native_profile_bonus_records_007fb340(void* p,NativeGameProfileLifetimeContext& c,Progress& state){
    if(void* cursor=pointer(p,4)){
        void* end=pointer(p,8);
        while(cursor!=end){state.cursor=cursor;state.native_site=0x007fb355;destroy_native_profile_three_strings_0061f630(cursor,c,state);cursor=at(cursor,0x1c);}
        state.native_site=0x007fb365;c.calls.free_00bf65ac(pointer(p,4));
    }
    word(p,4,0);word(p,8,0);word(p,0xc,0);
}
void destroy_native_game_race_record_007ff9f0(void* p,NativeGameProfileLifetimeContext& c,Progress& state){word(p,0,0x00d08d20);release(p,8,0x007ffa07,0x007ffa0e,c,state);}
void destroy_native_game_profile_007fd8a0(void* p,NativeGameProfileLifetimeContext& c,NativeGameProfileLifetimeOperation& o){
    using Op=NativeGameProfileLifetimeOperation;if(o.phase!=Op::Phase::fresh)throw std::logic_error("native profile destruction is one-shot");
    o.phase=Op::Phase::running;o.owner=p;o.context=&c;auto& calls=c.calls;
    try{
        word(p,0,0x00d08d1c);void* mission=pointer(p,0x64);o.unwind_state=0xd;
        if(mission){o.native_site=0x007fd8da;calls.profile_call_007fd780(mission,c,o.mission);o.native_site=0x007fd8e0;calls.free_00bf65ac(mission);word(p,0x64,0);}
        void* aliases=at(p,0xcc);o.unwind_state=0xc;o.native_site=0x007fd8f8;calls.profile_call_004d05e0(aliases,c);
        o.native_site=0x007fd901;calls.free_00bf65ac(pointer(aliases,4));word(aliases,4,0);
        o.unwind_state=0xb;o.native_site=0x007fd917;destroy_native_profile_bonus_records_007fb340(at(p,0xbc),c,o);
        void* begin=pointer(p,0xb0);void* strings=at(p,0xac);o.unwind_state=0xa;
        if(begin){void* end=pointer(strings,8);o.native_site=0x007fd93a;calls.profile_call_00432050(begin,end,c);o.native_site=0x007fd943;calls.free_00bf65ac(pointer(strings,4));}
        word(strings,4,0);word(strings,8,0);word(strings,0xc,0);
        using Range=void*(NativeGameProfileLifetimeCalls::*)(void*,void*,void*,void*,void*,void*,NativeGameProfileLifetimeContext&);
        const auto tree=[&](U offset,std::int32_t state,U erase_site,U free_site,Range erase){
            void* head=pointer(p,offset+4);void* first=pointer(head);void* receiver=at(p,offset);o.unwind_state=state;o.native_site=erase_site;
            (calls.*erase)(receiver,o.iterator_output,receiver,first,receiver,head,c);
            o.native_site=free_site;calls.free_00bf65ac(pointer(receiver,4));word(receiver,4,0);word(receiver,8,0);
        };
        tree(0xa0,9,0x007fd972,0x007fd97b,&NativeGameProfileLifetimeCalls::profile_call_0058d860);
        tree(0x94,8,0x007fd9a7,0x007fd9b0,&NativeGameProfileLifetimeCalls::profile_call_007fb230);
        tree(0x88,7,0x007fd9dc,0x007fd9e5,&NativeGameProfileLifetimeCalls::profile_call_004d1a50);
        tree(0x7c,6,0x007fda0e,0x007fda17,&NativeGameProfileLifetimeCalls::profile_call_004d1a50);
        tree(0x70,5,0x007fda3d,0x007fda46,&NativeGameProfileLifetimeCalls::profile_call_004d1a50);
        o.unwind_state=4;release(p,0x68,0x007fda6a,0x007fda71,c,o);
        o.unwind_state=3;release(p,0x50,0x007fda8c,0x007fda93,c,o);
        o.unwind_state=2;release(p,0x3c,0x007fdaae,0x007fdab5,c,o);
        o.unwind_state=1;release(p,0x34,0x007fdad0,0x007fdad7,c,o);
        o.native_site=0x007fdadf;calls.profile_call_007f8310(at(p,0x14));
        aliases=at(p,8);o.unwind_state=-1;o.native_site=0x007fdaf1;calls.profile_call_004d05e0(aliases,c);
        o.native_site=0x007fdafa;calls.free_00bf65ac(pointer(aliases,4));word(aliases,4,0);o.phase=Op::Phase::complete;
    }catch(...){o.phase=Op::Phase::failed;throw;}
}
} // namespace bsp
