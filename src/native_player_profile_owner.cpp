#include "bsp/native_player_profile_owner.hpp"
#include "bsp/native_profile_counter_map.hpp"
#include "bsp/native_checked_string_storage.hpp"
#include "bsp/native_fileblock_gate_list.hpp"
#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_render_resource_record.hpp"
#include "bsp/native_vfs_container_allocation.hpp"
#include "bsp/native_vfs_string_tree.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native player profile construction requires MSVC Win32.
#endif

namespace bsp {
void* NativePlayerProfileCalls::call_004c3020(){return allocate_native_render_alias_sentinel_004c3020();}
void* NativePlayerProfileCalls::call_007f82f0(){return allocate_native_list_head_007f82f0();}
void* NativePlayerProfileCalls::call_004c26b0(){return allocate_native_tree_node_004c26b0();}
void* NativePlayerProfileCalls::call_007f8540(){return allocate_native_profile_transient_node_007f8540(*this);}
void* NativePlayerProfileCalls::call_005826b0(){return allocate_native_profile_counter_node_005826b0(*this);}
void NativePlayerProfileCalls::call_004cec60(void* tree,void* node,NativeStringStorage& strings){
    erase_native_vfs_string_subtree_004cec60(tree,node,strings);
}
void NativePlayerProfileCalls::call_0058b520(void* tree,void* node,NativeStringStorage& strings){
    erase_native_profile_counter_subtree_0058b520(tree,node,strings,*this);
}
void NativePlayerProfileCalls::call_007fa880(void* tree,void* node,NativeStringStorage& strings){
    erase_native_profile_transient_subtree_007fa880(tree,node,strings,*this);
}
std::uint32_t* NativePlayerProfileCalls::call_005070c0(void* tree,const void* key,NativeStringStorage& strings){
    return index_native_profile_counter_005070c0(tree,key,strings,*this);
}
void NativePlayerProfileCalls::call_008d4820(void* settings,NativeProfileSettingsContext& context){
    reset_native_profile_control_defaults_008d4820(settings,context,*this);
}
void NativePlayerProfileCalls::call_008d41c0(void* settings){reset_native_profile_game_defaults_008d41c0(settings);}
void NativePlayerProfileCalls::call_004d05e0(void* list,NativeStringStorage& strings){
    clear_native_render_resource_aliases_004d05e0(list,strings);
}
void NativePlayerProfileCalls::call_00bf6713(){_invalid_parameter_noinfo();}
void* NativePlayerProfileCalls::call_004954f0(void* vector,void* output,void* first_owner,
    void* first,void* last_owner,void* last,NativeStringStorage& strings){
    return erase_checked_native_string_storage(vector,output,first_owner,first,last_owner,last,strings);
}
void* NativePlayerProfileCalls::call_007f8390(void* next,void* previous,const std::uint8_t* gate){
    return allocate_native_fileblock_gate_node_007f8390(next,previous,gate);
}
std::uint32_t NativePlayerProfileCalls::call_007fa3a0(void* list,std::uint32_t n){
    return grow_native_fileblock_gate_count_007fa3a0(list,n);
}
namespace {
using Word=std::uint32_t;
using Op=NativePlayerProfileOperation;
static_assert(sizeof(void*)==4&&sizeof(NativeString)==8);
void* at(void* p,Word n) noexcept {return static_cast<std::byte*>(p)+n;}
const void* at(const void* p,Word n) noexcept {return static_cast<const std::byte*>(p)+n;}
template<class T>T read(const void* p,Word n=0) noexcept {return *static_cast<const volatile T*>(at(p,n));}
template<class T>void put(void* p,Word n,T value) noexcept {*static_cast<volatile T*>(at(p,n))=value;}
void word(void* p,Word n,Word value=0) noexcept {put(p,n,value);}
void* pointer(Word value) noexcept {return reinterpret_cast<void*>(value);}
void sentinel(void* header,void* allocation,Word nil_offset) noexcept {
    put(header,4,allocation);put<std::uint8_t>(allocation,nil_offset,1);
    void* head=read<void*>(header,4);put(head,4,head);
    head=read<void*>(header,4);put(head,0,head);
    head=read<void*>(header,4);put(head,8,head);word(header,8);
}
void reset_head(void* tree) noexcept {
    void* head=read<void*>(tree,4);put(head,4,head);
    head=read<void*>(tree,4);word(tree,8);put(head,0,head);
    head=read<void*>(tree,4);put(head,8,head);
}
void* root(void* tree) noexcept {return read<void*>(read<void*>(tree,4),4);}
void literal(void* header,Word length,const char* text,Word resize_site,Word copy_site,
    NativePlayerProfileContext& c,Op& a) {
    a.native_site=resize_site;resize_native_string_header_0041dd40(header,c.strings,length,false);
    void* const target=read<void*>(header,4);
    if(target){const Word bytes=read<Word>(header);a.native_site=copy_site;std::memmove(target,text,bytes);}
}
void reset_body(void* profile,NativePlayerProfileContext& c,Op& a) {
    auto& calls=c.calls;
    a.native_site=0x007fdb42;
    const void* const empty=construct_native_profile_empty_name_00436710(a.temporary,c.strings,c.empty_00ce3a0c);
    a.unwind_state=0;void* header=at(profile,0x34);
    if(header!=empty){
        a.native_site=0x007fdb5d;resize_native_string_header_0041dd40(header,c.strings,read<Word>(empty),true);
        if(read<Word>(empty)!=0){const Word bytes=read<Word>(header);void* const target=read<void*>(header,4);
            const void* const source=read<void*>(empty,4);a.native_site=0x007fdb72;std::memmove(target,source,bytes);}
    }
    a.unwind_state=-1;
    if(char* const data=read<char*>(&a.temporary,4)){const Word bytes=read<Word>(&a.temporary)+1;
        a.native_site=0x007fdb98;c.strings.release(data,bytes);}
    literal(at(profile,0x3c),0x11,c.new_player_00cef794,0x007fdbac,0x007fdbc1,c,a);
    literal(at(profile,0x50),0,c.empty_00ce3a0c,0x007fdbd0,0x007fdbe5,c,a);
    put<std::uint8_t>(profile,0x59,1);word(profile,0x60,1);word(profile,0x5c,1);
    void* const progress=read<void*>(profile,0x64);
    if(progress){a.native_site=0x007fdc00;
        destroy_native_mission_progress_007fd780(progress,c.strings,calls,a.progress_destruction);
        a.native_site=0x007fdc06;calls.free_00bf65ac(progress);word(profile,0x64);}
    a.native_site=0x007fdc13;a.current_allocation=calls.allocate_00bf681b(0x24);
    a.unwind_state=1;void* value=nullptr;
    if(a.current_allocation){a.native_site=0x007fdc29;
        value=construct_native_mission_progress_00920e10(a.current_allocation,calls,a.progress_construction);}
    a.unwind_state=-1;put(profile,0x64,value);
    literal(at(profile,0x68),0,c.empty_00ce3a0c,0x007fdc44,0x007fdc59,c,a);
    header=at(profile,0x70);a.native_site=0x007fdc6d;calls.call_004cec60(header,root(header),c.strings);reset_head(header);
    header=at(profile,0x7c);a.native_site=0x007fdc95;calls.call_004cec60(header,root(header),c.strings);reset_head(header);
    header=at(profile,0x88);a.native_site=0x007fdcc0;calls.call_004cec60(header,root(header),c.strings);reset_head(header);
    void* const counters=at(profile,0xa0);
    a.native_site=0x007fdceb;calls.call_0058b520(counters,root(counters),c.strings);reset_head(counters);
    header=at(profile,0xac);a.captured_last=read<Word>(header,8);
    if(read<Word>(header,4)>a.captured_last){a.native_site=0x007fdd16;calls.call_00bf6713();}
    a.captured_first=read<Word>(header,4);
    if(a.captured_first>read<Word>(header,8)){a.native_site=0x007fdd27;calls.call_00bf6713();}
    a.native_site=0x007fdd3f;
    calls.call_004954f0(header,a.iterator_output,header,pointer(a.captured_first),header,pointer(a.captured_last),c.strings);
    header=at(profile,0x94);a.native_site=0x007fdd56;calls.call_007fa880(header,root(header),c.strings);reset_head(header);
    word(profile,0xe0,1);word(profile,0xe4,1);
    a.native_site=0x007fdd89;calls.call_0058b520(counters,root(counters),c.strings);reset_head(counters);
    word(&a.temporary,0);word(&a.temporary,4);
    a.native_site=0x007fddb1;resize_native_string_header_0041dd40(&a.temporary,c.strings,4,true);
    a.captured_rank_buffer=read<char*>(&a.temporary,4);
    if(a.captured_rank_buffer){const Word bytes=read<Word>(&a.temporary)+1;a.native_site=0x007fddcc;
        std::memmove(a.captured_rank_buffer,c.rank_00cef15c,bytes);}
    a.unwind_state=2;a.native_site=0x007fdde3;
    put<Word>(calls.call_005070c0(counters,&a.temporary,c.strings),0,1);a.unwind_state=-1;
    if(a.captured_rank_buffer){const Word bytes=read<Word>(&a.temporary)+1;a.native_site=0x007fde05;
        c.strings.release(a.captured_rank_buffer,bytes);}
    a.native_site=0x007fde14;calls.call_004d05e0(at(profile,8),c.strings);
    word(profile,0x20);word(profile,0x24);word(profile,0x28);word(profile,0x2c,9);
    header=at(profile,0x14);void* head=read<void*>(header,4);void* node=read<void*>(head);
    put(head,0,head);head=read<void*>(header,4);put(head,4,head);word(header,8);
    while(node!=read<void*>(header,4)){
        void* const next=read<void*>(node);a.native_site=0x007fde44;calls.free_00bf65ac(node);node=next;
    }
    a.lobby_index=0;
    do {
        head=read<void*>(header,4);void* const previous=read<void*>(head,4);a.gate=a.lobby_index<5?1:0;
        a.native_site=0x007fde7b;node=calls.call_007f8390(head,previous,&a.gate);
        a.current_allocation=node;a.native_site=0x007fde88;calls.call_007fa3a0(header,1);
        put(head,4,node);put(read<void*>(node,4),0,node);++a.lobby_index;
    }while(a.lobby_index<9);
    word(profile,0x30,0xffffffff);word(profile,0xf0);word(profile,0xec);
    a.native_site=0x007fdec2;calls.call_004d05e0(at(profile,0xcc),c.strings);
    word(profile,0xd8);word(profile,0xdc);
    a.native_site=0x007fded8;calls.call_008d4820(c.actual_settings_00f88980,c.settings);
    a.native_site=0x007fdee2;calls.call_008d41c0(c.actual_settings_00f88980);
}
void enter(void* owner,NativePlayerProfileContext& c,Op& a,bool construction){
    if(a.phase!=Op::Phase::fresh)throw std::logic_error("profile operation is one-shot");
    a.phase=Op::Phase::running;a.owner=owner;a.context=&c;a.construction=construction;
}
} // namespace
NativePlayerProfileOperation::~NativePlayerProfileOperation(){
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void NativePlayerProfileOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::failed){
        progress_destruction.acknowledge_diagnostic_cleanup();
        progress_construction.acknowledge_diagnostic_cleanup();
        phase=Phase::diagnostic_retired;
    }
}
NativeString* construct_native_profile_empty_name_00436710(NativeString& output,
    NativeStringStorage& strings,const char* empty){output.assign_0041e870(strings,empty);return &output;}
void reset_native_player_profile_007fdb20(void* profile,NativePlayerProfileContext& c,Op& a){
    enter(profile,c,a,false);
    try{reset_body(profile,c,a);a.phase=Op::Phase::complete;}
    catch(...){a.phase=Op::Phase::failed;throw;}
}
void* construct_native_player_profile_007fee20(void* profile,NativePlayerProfileContext& c,Op& a){
    enter(profile,c,a,true);auto& calls=c.calls;
    try{
        word(profile,0,0x00d08d1c);a.constructor_site=0x007fee4b;
        put(profile,0x0c,calls.call_004c3020());word(profile,0x10);
        a.constructor_unwind_state=0;a.constructor_site=0x007fee61;
        put(profile,0x18,calls.call_007f82f0());word(profile,0x1c);
        word(profile,0x34);word(profile,0x38);word(profile,0x3c);word(profile,0x40);
        word(profile,0x50);word(profile,0x54);word(profile,0x68);word(profile,0x6c);
        a.constructor_unwind_state=5;a.constructor_site=0x007fee8e;
        sentinel(at(profile,0x70),calls.call_004c26b0(),0x15);
        a.constructor_unwind_state=6;a.constructor_site=0x007feeb9;
        sentinel(at(profile,0x7c),calls.call_004c26b0(),0x15);
        a.constructor_unwind_state=7;a.constructor_site=0x007feee5;
        sentinel(at(profile,0x88),calls.call_004c26b0(),0x15);
        a.constructor_unwind_state=8;a.constructor_site=0x007fef11;
        sentinel(at(profile,0x94),calls.call_007f8540(),0x29);
        a.constructor_unwind_state=9;a.constructor_site=0x007fef3d;
        sentinel(at(profile,0xa0),calls.call_005826b0(),0x19);
        word(profile,0xb0);word(profile,0xb4);word(profile,0xb8);
        word(profile,0xc0);word(profile,0xc4);word(profile,0xc8);
        a.constructor_unwind_state=12;a.constructor_site=0x007fef8d;
        put(profile,0xd0,calls.call_004c3020());word(profile,0xd4);
        a.constructor_unwind_state=13;a.constructor_site=0x007fef9f;
        reset_body(profile,c,a);a.phase=Op::Phase::complete;return profile;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
} // namespace bsp
