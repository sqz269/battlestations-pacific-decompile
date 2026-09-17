#include "bsp/native_profile_collections.hpp"
#include "bsp/native_mission_score_record.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native profile collections require MSVC Win32.
#endif

namespace bsp {
void* NativeProfileCollectionCalls::allocate_00bf681b(std::uint32_t n){
    return singleton_lifetime_allocate({SingletonAllocationKind::object,n,n});
}
void NativeProfileCollectionCalls::free_00bf65ac(void* p){singleton_lifetime_free(p);}
void NativeProfileCollectionCalls::call_00593570(void* record,NativeStringStorage& strings){
    destroy_native_mission_score_record_00593570(record,strings,*this);
}
namespace {
using Word=std::uint32_t;
static_assert(sizeof(void*)==4);
Word address(const void* p) noexcept {return reinterpret_cast<Word>(p);}
void* at(void* p,Word n) noexcept {return reinterpret_cast<void*>(address(p)+n);}
const void* at(const void* p,Word n) noexcept {return reinterpret_cast<const void*>(address(p)+n);}
template<class T>T read(const void* p,Word n=0) noexcept {return *static_cast<const volatile T*>(at(p,n));}
template<class T>void put(void* p,Word n,T value) noexcept {*static_cast<volatile T*>(at(p,n))=value;}
void* blank_node(Word bytes,Word color,NativeProfileCollectionCalls& calls){
    void* const p=calls.allocate_00bf681b(bytes);
    // Original leaves use independent wrapped-address guards, including their
    // later fault when a returning allocator supplies null. No early success.
    for(Word n=0;n<=8;n+=4)if(address(p)+n!=0)put<Word>(p,n,0);
    put<std::uint8_t>(p,color,1);put<std::uint8_t>(p,color+1,0);return p;
}
bool nil(const void* node,Word offset) noexcept {return read<std::uint8_t>(node,offset)!=0;}
void* finish_full_range(void* tree,void* output) noexcept {
    void* head=read<void*>(tree,4);put(head,4,head);
    head=read<void*>(tree,4);put<Word>(tree,8,0);put(head,0,head);
    head=read<void*>(tree,4);put(head,8,head);
    void* const first=read<void*>(read<void*>(tree,4));
    put(output,0,tree);put(output,4,first);return output;
}
}
void* allocate_native_profile_counter_node_005826b0(NativeProfileCollectionCalls& c){return blank_node(0x1c,0x18,c);}
void* allocate_native_profile_transient_node_007f8540(NativeProfileCollectionCalls& c){return blank_node(0x2c,0x28,c);}
void* allocate_native_mission_score_node_00907ca0(NativeProfileCollectionCalls& c){return blank_node(0x2a0,0x29c,c);}
void erase_native_profile_counter_subtree_0058b520(void* tree,void* node,
    NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    while(!nil(node,0x19)){
        erase_native_profile_counter_subtree_0058b520(tree,read<void*>(node,8),strings,calls);
        char* const data=read<char*>(node,0x10);void* const next=read<void*>(node);
        if(data){const Word bytes=read<Word>(node,0xc)+1;strings.release(data,bytes);}
        calls.free_00bf65ac(node);node=next;
    }
}
void destroy_native_profile_transient_pair_007f89f0(void* pair,NativeStringStorage& strings){
    if(char* const data=read<char*>(pair,0x18)){const Word bytes=read<Word>(pair,0x14)+1;strings.release(data,bytes);}
    if(char* const data=read<char*>(pair,4)){const Word bytes=read<Word>(pair)+1;strings.release(data,bytes);}
}
void erase_native_profile_transient_subtree_007fa880(void* tree,void* node,
    NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    while(!nil(node,0x29)){
        erase_native_profile_transient_subtree_007fa880(tree,read<void*>(node,8),strings,calls);
        void* const next=read<void*>(node);
        destroy_native_profile_transient_pair_007f89f0(at(node,0xc),strings);
        calls.free_00bf65ac(node);node=next;
    }
}
void erase_native_mission_score_subtree_007fd510(void* tree,void* node,
    NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    while(!nil(node,0x29d)){
        erase_native_mission_score_subtree_007fd510(tree,read<void*>(node,8),strings,calls);
        void* const next=read<void*>(node);
        calls.call_00593570(at(node,0x14),strings);
        if(char* const data=read<char*>(node,0x10)){const Word bytes=read<Word>(node,0xc)+1;strings.release(data,bytes);}
        calls.free_00bf65ac(node);node=next;
    }
}
void* clear_native_profile_counter_full_range_0058d860(void* tree,void* output,
    NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    erase_native_profile_counter_subtree_0058b520(tree,read<void*>(read<void*>(tree,4),4),strings,calls);
    return finish_full_range(tree,output);
}
void* clear_native_mission_score_full_range_007fd5f0(void* tree,void* output,
    NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    erase_native_mission_score_subtree_007fd510(tree,read<void*>(read<void*>(tree,4),4),strings,calls);
    return finish_full_range(tree,output);
}
} // namespace bsp
