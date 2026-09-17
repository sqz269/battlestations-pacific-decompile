#include "bsp/detail/native_blank_container_storage.hpp"
#include "bsp/native_game_array_elements.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native game array elements require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
using Stage=NativeGameArrayElementStage;
using Op=NativeGameArrayOperation;
static_assert(sizeof(void*)==4);
void* at(void* p,Word n) noexcept {return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+n);}
const void* at(const void* p,Word n) noexcept {return reinterpret_cast<const void*>(reinterpret_cast<Word>(p)+n);}
template<class T>T read(const void* p,Word n=0) noexcept {return *static_cast<const volatile T*>(at(p,n));}
template<class T>void put(void* p,Word n,T v) noexcept {*static_cast<volatile T*>(at(p,n))=v;}
void word(void* p,Word n,Word v=0) noexcept {put(p,n,v);}
void byte(void* p,Word n,std::uint8_t v=0) noexcept {put(p,n,v);}
void* list_head(Word size,NativeGameArrayCalls& calls,Stage& stage,Word site){
    stage.storage_site=site;
    return detail::allocate_self_linked_list_storage(size,[&](Word n){return calls.allocate_00bf681b(n);});
}
struct CompletedMessage {
    NativeLegacySboStringStorage& value;
    ~CompletedMessage() noexcept {native_legacy_sbo_string_destroy_004072d0(value);}
};
[[noreturn]]void too_long(){
    NativeLegacySboStringStorage text;text.capacity_18=15;text.length_14=0;text.buffer_04.inline_bytes[0]=0;
    native_legacy_sbo_string_assign_counted_00408720(text,"list<T> too long",16);
    const CompletedMessage completed{text};throw NativeHardwareLayoutTreeLengthError{text};
}
void grow_count(void* list,Word increment){
    const Word count=read<Word>(list,8);
    if(Word{0x1fffffff}-count<increment)too_long();
    word(list,8,count+increment);
}
void* pair_node(void* next,void* previous,const void* pair,NativeGameArrayCalls& calls,Stage& stage){
    stage.storage_site=0x004c3702;void* const node=calls.allocate_00bf681b(0x10);stage.pending_node=node;
    if(node)put(node,0,next);
    void* const link=at(node,4);if(link)put(link,0,previous);
    void* const value=at(node,8);
    if(value){word(value,0,read<Word>(pair));word(value,4,read<Word>(pair,4));}
    return node;
}
} // namespace
void* NativeGameArrayCalls::allocate_00bf681b(Word n){return singleton_lifetime_allocate({SingletonAllocationKind::object,n,n});}
void NativeGameArrayCalls::construct_empty_strings_00bf7cd1(void* p,Word stride,Word count,Word ctor,Word dtor){
    if(stride!=8||ctor!=0x00415270||dtor!=0x0041dd20)throw std::invalid_argument("unsupported native string-array contract");
    for(Word i=0;i<count;++i)initialize_native_string_header_00415270(at(p,i*stride));
}
NativeGameArrayOperation::~NativeGameArrayOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void NativeGameArrayOperation::acknowledge_diagnostic_cleanup() noexcept {if(phase==Phase::failed)phase=Phase::diagnostic_retired;}
void* initialize_native_game_vector_004d3730(void* p) noexcept {word(p,4);word(p,8);word(p,0xc);return p;}
void* construct_native_game_list_004c8140(void* p,NativeGameArrayCalls& calls,Stage& stage){
    stage.owner=p;stage.native_site=0x004c8143;
    put(p,4,list_head(0xc,calls,stage,0x004c1972));word(p,8);return p;
}
void insert_native_game_participant_pairs_004d2920(void* list,Word count,const void* pair,
    NativeGameArrayCalls& calls,Stage& stage){
    // 4D2920 captures head->next even when count0;4D0D50 keeps that position.
    void* const position=read<void*>(read<void*>(list,4));stage.storage_site=0x004d295a;
    while(count!=0){
        void* const previous=read<void*>(position,4);
        void* const node=pair_node(position,previous,pair,calls,stage);
        stage.storage_site=0x004d0da0;grow_count(list,1);
        put(position,4,node);void* const current_previous=read<void*>(node,4);
        --count;put(current_previous,0,node);stage.pending_node=nullptr;
    }
}
void* construct_native_game_participant_004d6ba0(void* p,const NativeGameArrayConstants& c,
    NativeGameArrayCalls& calls,Stage& stage){
    stage.owner=p;stage.unwind_state=-1;
    word(p,0,0x00ce7794);put<std::uint16_t>(p,0x10,0xfffd);byte(p,0x1a);word(p,0x2c,3);
    word(p,0x14,2);word(p,0x28,2);word(p,0x3c);word(p,0x40);word(p,0x44);word(p,0x4c);
    byte(p,0x48,1);word(p,0x38,0x00ce75cc);const Word bits=c.bits_00ce4adc;
    word(p,0x50);byte(p,0x88);byte(p,0x89,1);word(p,0x8c,bits);byte(p,0x90);word(p,0x94,3);
    stage.unwind_state=0;void* const list=at(p,0x98);word(list,8);Word pair[2]={0,0};
    stage.native_site=0x004d6c42;put(list,4,list_head(0x10,calls,stage,0x004c3182));
    stage.native_site=0x004d6c52;insert_native_game_participant_pairs_004d2920(list,0,pair,calls,stage);
    stage.unwind_state=1;word(p,0xa4);word(p,0xa8,0xffffffc4);byte(p,0xc8);
    stage.native_site=0x004d6c87;calls.construct_empty_strings_00bf7cd1(at(p,0xec),8,3,0x00415270,0x0041dd20);
    word(p,0x110,0xffffffff);word(p,0xe0);word(p,0xe4);word(p,0xe8);
    word(p,0x104);word(p,0x108);word(p,0x10c);byte(p,0x78);return p;
}
void construct_native_game_array_00bf7cd1(void* p,Word stride,Word count,Word ctor,Word dtor,
    const NativeGameArrayConstants& c,NativeGameArrayCalls& calls,Op& a){
    if(a.phase!=Op::Phase::fresh)throw std::logic_error("game array construction is one-shot");
    const bool vector=ctor==0x004d3730&&dtor==0x004cafd0&&stride==0x10;
    const bool participant=ctor==0x004d6ba0&&dtor==0x004cb2f0&&stride==0x118;
    const bool list=ctor==0x004c8140&&dtor==0x004c4600&&stride==0xc;
    if(!vector&&!participant&&!list)throw std::invalid_argument("unsupported native game-array contract");
    a.base=p;a.stride=stride;a.count=count;a.constructor=ctor;a.destructor=dtor;a.phase=Op::Phase::running;
    try{
        while(a.completed<count){
            void* const element=at(p,a.completed*stride);a.element={};a.element.owner=element;
            if(vector)initialize_native_game_vector_004d3730(element);
            else if(participant)construct_native_game_participant_004d6ba0(element,c,calls,a.element);
            else construct_native_game_list_004c8140(element,calls,a.element);
            ++a.completed;
        }
        a.phase=Op::Phase::complete;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
} // namespace bsp
