#include "bsp/native_mission_progress_owner.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Word=std::uint32_t;
using Op=NativeMissionProgressOperation;
static_assert(sizeof(void*)==4);
void* at(void* p,Word n) noexcept {return static_cast<std::byte*>(p)+n;}
template<class T>T read(void* p,Word n=0) noexcept {return *static_cast<volatile T*>(at(p,n));}
template<class T>void put(void* p,Word n,T value) noexcept {*static_cast<volatile T*>(at(p,n))=value;}
void sentinel(void* tree,void* node,Word nil_offset) noexcept {
    put(tree,4,node);put<std::uint8_t>(node,nil_offset,1);
    node=read<void*>(tree,4);put(node,4,node);
    node=read<void*>(tree,4);put(node,0,node);
    node=read<void*>(tree,4);put(node,8,node);put<Word>(tree,8,0);
}
void enter(void* owner,NativeProfileCollectionCalls& c,Op& a,bool destruction){
    if(a.phase!=Op::Phase::fresh)throw std::logic_error("mission progress operation is one-shot");
    a.phase=Op::Phase::running;a.owner=owner;a.calls=&c;a.destruction=destruction;
}
}
NativeMissionProgressOperation::~NativeMissionProgressOperation(){
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void NativeMissionProgressOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::failed)phase=Phase::diagnostic_retired;
}
void* construct_native_mission_progress_00920e10(void* owner,NativeProfileCollectionCalls& c,Op& a){
    enter(owner,c,a,false);
    try{
        a.native_site=0x00920e30;sentinel(owner,allocate_native_mission_score_node_00907ca0(c),0x29d);
        a.unwind_state=0;a.native_site=0x00920e5f;
        sentinel(at(owner,0xc),allocate_native_profile_counter_node_005826b0(c),0x19);
        a.unwind_state=1;a.native_site=0x00920e87;
        sentinel(at(owner,0x18),allocate_native_profile_counter_node_005826b0(c),0x19);
        a.phase=Op::Phase::complete;return owner;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
void destroy_native_mission_progress_007fd780(void* owner,NativeStringStorage& strings,
    NativeProfileCollectionCalls& c,Op& a){
    enter(owner,c,a,true);a.strings=&strings;
    try{
        void* tree=at(owner,0x18);a.unwind_state=1;a.native_site=0x007fd7bc;
        clear_native_profile_counter_full_range_0058d860(tree,a.iterator_output,strings,c);
        a.native_site=0x007fd7c5;c.free_00bf65ac(read<void*>(tree,4));
        put<Word>(tree,4,0);put<Word>(tree,8,0);
        tree=at(owner,0xc);a.unwind_state=0;a.native_site=0x007fd7ec;
        clear_native_profile_counter_full_range_0058d860(tree,a.iterator_output,strings,c);
        a.native_site=0x007fd7f5;c.free_00bf65ac(read<void*>(tree,4));
        put<Word>(tree,4,0);put<Word>(tree,8,0);
        a.unwind_state=-1;a.native_site=0x007fd81b;
        clear_native_mission_score_full_range_007fd5f0(owner,a.iterator_output,strings,c);
        a.native_site=0x007fd824;c.free_00bf65ac(read<void*>(owner,4));
        put<Word>(owner,4,0);put<Word>(owner,8,0);a.phase=Op::Phase::complete;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
} // namespace bsp
