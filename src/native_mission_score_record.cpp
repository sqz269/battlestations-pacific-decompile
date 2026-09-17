#include "bsp/native_mission_score_record.hpp"
#include <cstdint>
#include <initializer_list>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission score storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
static_assert(sizeof(void*)==4);
void* at(void* p,Word n) noexcept {return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+n);}
template<class T>T read(void* p,Word n=0) noexcept {return *static_cast<volatile T*>(at(p,n));}
template<class T>void put(void* p,Word n,T value) noexcept {*static_cast<volatile T*>(at(p,n))=value;}

// Native storage shapes only. The original functions implement checked range
// operations too; here each caller supplies its own complete current range.
enum class Tree {strings,scalar,wide_scalar,objective,nested,nested_alt,damage,kills};
void release_string(void* header,NativeStringStorage& strings){
    if(char* const p=read<char*>(header,4)){
        const Word bytes=read<Word>(header)+1;strings.release(p,bytes);
    }
}
void clear_range(void*,Tree,NativeStringStorage&,NativeProfileCollectionCalls&);
void destroy_tree(void* tree,Tree kind,NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    clear_range(tree,kind,strings,calls);
    calls.free_00bf65ac(read<void*>(tree,4));
    put<Word>(tree,4,0);put<Word>(tree,8,0);
}
void erase_subtree(void* tree,void* node,Tree kind,
    NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    const Word nil=kind==Tree::wide_scalar?0x3d:kind==Tree::objective?0x21:
        (kind==Tree::nested||kind==Tree::nested_alt||kind==Tree::damage||kind==Tree::kills)?0x1d:0x15;
    while(read<std::uint8_t>(node,nil)==0){
        erase_subtree(tree,read<void*>(node,8),kind,strings,calls);
        void* const next=read<void*>(node);
        if(kind==Tree::objective){
            // 58BCD0 -> 585FF0(node+C): second NativeString then first.
            release_string(at(node,0x14),strings);release_string(at(node,0xc),strings);
        }else if(kind==Tree::nested||kind==Tree::nested_alt){
            // 5914B0/591800 -> 58AD40(node+10), current-head free and clear.
            destroy_tree(at(node,0x10),Tree::scalar,strings,calls);
        }else if(kind==Tree::damage){
            // 592A20 -> 591A30 -> 5914B0.
            destroy_tree(at(node,0x10),Tree::nested,strings,calls);
        }else if(kind==Tree::kills){
            // 592EA0 -> 591C00 -> 591800.
            destroy_tree(at(node,0x10),Tree::nested_alt,strings,calls);
        }
        calls.free_00bf65ac(node);node=next;
    }
}
void clear_range(void* tree,Tree kind,NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    if(kind==Tree::strings){
        Word output[2];clear_native_profile_counter_full_range_0058d860(tree,output,strings,calls);return;
    }
    // 58AD40/58AE10/58AF30/58B000: scalar24;58B0D0: scalar64;
    // 58F100: objective;591A30/591C00: nested;592F10/593180: triples.
    erase_subtree(tree,read<void*>(read<void*>(tree,4),4),kind,strings,calls);
    void* head=read<void*>(tree,4);put(head,4,head);
    head=read<void*>(tree,4);put<Word>(tree,8,0);put(head,0,head);
    head=read<void*>(tree,4);put(head,8,head);
    // Native output iterator lives in an unobserved caller-local scratch slot.
    // The public boundary does not expose native stack aliases or iterator ABI.
}
void clear_plain_vector(void* begin,NativeProfileCollectionCalls& calls){
    if(void* const allocation=read<void*>(begin))calls.free_00bf65ac(allocation);
    put<Word>(begin,0,0);put<Word>(begin,4,0);put<Word>(begin,8,0);
}
} // namespace

void destroy_native_mission_score_record_00593570(void* record,
    NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    release_string(at(record,0x264),strings);
    release_string(at(record,0x25c),strings);
    release_string(at(record,0x254),strings);
    for(Word offset:{0x240u,0x234u,0x228u,0x21cu,0x210u,0x204u})
        destroy_tree(at(record,offset),Tree::objective,strings,calls);
    destroy_tree(at(record,0x1f8),Tree::strings,strings,calls);
    destroy_tree(at(record,0x1ec),Tree::strings,strings,calls);
    destroy_tree(at(record,0x1bc),Tree::wide_scalar,strings,calls);
    for(Word offset:{0x1b0u,0x1a4u,0x198u,0x15cu})
        destroy_tree(at(record,offset),Tree::scalar,strings,calls);
    for(Word offset:{0x150u,0x140u,0x130u})clear_plain_vector(at(record,offset),calls);
    destroy_tree(at(record,0x120),Tree::strings,strings,calls);
    destroy_tree(at(record,0x114),Tree::scalar,strings,calls);
    for(Word offset:{0x108u,0xfcu,0xf0u,0xe4u,0xd8u})
        destroy_tree(at(record,offset),Tree::scalar,strings,calls);
    destroy_tree(at(record,0xcc),Tree::nested,strings,calls);
    destroy_tree(at(record,0xc0),Tree::kills,strings,calls);
    destroy_tree(at(record,0xb4),Tree::kills,strings,calls);
    destroy_tree(at(record,0xa8),Tree::damage,strings,calls);
    destroy_tree(at(record,0x9c),Tree::damage,strings,calls);
    destroy_tree(at(record,0x90),Tree::strings,strings,calls);
    destroy_tree(at(record,0x84),Tree::strings,strings,calls);
    // Final nine calls are native590B40 wrappers around counter clear/free.
    for(Word offset:{0x78u,0x6cu,0x60u,0x54u,0x48u,0x3cu,0x30u,0x24u,0x18u})
        destroy_tree(at(record,offset),Tree::strings,strings,calls);
}
} // namespace bsp
