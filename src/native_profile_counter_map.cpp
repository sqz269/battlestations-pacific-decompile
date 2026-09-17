#include "bsp/native_profile_counter_map.hpp"
#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native profile counter maps require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
using A=detail::TreeInsertAccess<0x18,0x19>;
struct Iterator {void* owner;void* node;};
struct InsertResult {Iterator iterator;std::uint8_t inserted;};
void* at(void* p,Word n) noexcept {return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+n);}
const void* at(const void* p,Word n) noexcept {return reinterpret_cast<const void*>(reinterpret_cast<Word>(p)+n);}
template<class T>T read(const void* p,Word n=0) noexcept {return *static_cast<const volatile T*>(at(p,n));}
template<class T>void put(void* p,Word n,T value) noexcept {*static_cast<volatile T*>(at(p,n))=value;}
void invalid(){_invalid_parameter_noinfo();}
struct CompletedMessage {
    NativeLegacySboStringStorage& value;
    ~CompletedMessage() noexcept {native_legacy_sbo_string_destroy_004072d0(value);}
};
[[noreturn]]void too_long(){
    NativeLegacySboStringStorage text;text.capacity_18=15;text.length_14=0;text.buffer_04.inline_bytes[0]=0;
    native_legacy_sbo_string_assign_counted_00408720(text,"map/set<T> too long",19);
    const CompletedMessage completed{text};throw NativeHardwareLayoutTreeLengthError{text};
}
void copy_key(void* destination,const void* source,NativeStringStorage& strings){
    put<Word>(destination,0,0);put<Word>(destination,4,0);
    if(destination!=source){
        resize_native_string_header_0041dd40(destination,strings,read<Word>(source),true);
        if(read<Word>(source)!=0)std::memmove(read<void*>(destination,4),read<void*>(source,4),read<Word>(destination));
    }
}
void* make_node(void* left,void* parent,void* right,const void* pair,std::uint8_t color,
    NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    // 504BF3 calls BF681B directly; the SDK response calls its BF55BE thunk.
    void* const node=calls.allocate_00bf681b(0x1c);
    if(node){
        try{
            put(node,0,left);put(node,4,parent);put(node,8,right);
            copy_key(at(node,0xc),pair,strings);
            put<Word>(node,0x14,read<Word>(pair,8));
            put<std::uint8_t>(node,0x18,color);put<std::uint8_t>(node,0x19,0);
        }catch(...){calls.free_00bf65ac(node);throw;}
    }
    return node;
}
void rotate_right(void* tree,void* node) noexcept {
    // 4FA600. Left rotations reuse the existing equivalent storage template.
    void* const pivot=A::left(node);A::left(node)=A::right(pivot);
    if(!A::sentinel(A::right(pivot)))A::parent(A::right(pivot))=node;
    A::parent(pivot)=A::parent(node);
    if(node==A::parent(A::head(tree)))A::parent(A::head(tree))=pivot;
    else if(node==A::right(A::parent(node)))A::right(A::parent(node))=pivot;
    else A::left(A::parent(node))=pivot;
    A::right(pivot)=node;A::parent(node)=pivot;
}
Iterator* link_node(void* tree,Iterator* out,std::uint8_t left,void* parent,const void* pair,
    NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    detail::link_tree_node<A>(tree,out,left,parent,pair,0x15555554u,
        [&strings,&calls](void* l,void* p,void* r,const void* v,std::uint8_t c){return make_node(l,p,r,v,c,strings,calls);},
        &detail::rotate_left_inlined<A>,&rotate_right,&too_long);return out;
}
bool less(const void* a,const void* b) noexcept {return less_native_string_headers_00443d00(a,b);}
void increment(Iterator& i){
    if(!i.owner)invalid();void* node=i.node;
    if(A::sentinel(node)){invalid();return;}
    void* child=A::right(node);
    if(A::sentinel(child)){
        void* parent=A::parent(node);
        while(!A::sentinel(parent)&&i.node==A::right(parent)){i.node=parent;parent=A::parent(parent);}
        i.node=parent;return;
    }
    void* next=A::left(child);while(!A::sentinel(next)){child=next;next=A::left(child);}i.node=child;
}
InsertResult* unique(void* tree,InsertResult* out,const void* pair,
    NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    return detail::insert_unique_tree_pair<A,Iterator>(tree,out,pair,
        [pair](void* n){return less(pair,at(n,0xc));},[pair](void* n){return less(at(n,0xc),pair);},
        [](Iterator& i){detail::decrement_tree_iterator<A>(&i,&invalid);},
        [&strings,&calls](void* t,Iterator* o,std::uint8_t l,void* p,const void* v){return link_node(t,o,l,p,v,strings,calls);},
        [](InsertResult* o,void* owner,void* node,std::uint8_t flag){put(o,4,node);put<std::uint8_t>(o,8,flag);put(o,0,owner);});
}
Iterator* hint(void* tree,Iterator* out,Iterator position,const void* pair,
    NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    if(A::count(tree)==0)return link_node(tree,out,1,A::head(tree),pair,strings,calls);
    void* const minimum=A::left(A::head(tree));if(!position.owner||position.owner!=tree)invalid();
    if(position.node==minimum){
        if(less(pair,at(position.node,0xc)))return link_node(tree,out,1,position.node,pair,strings,calls);
    }else{
        void* const end=A::head(tree);if(!position.owner||position.owner!=tree)invalid();
        if(position.node==end){
            if(less(at(A::right(A::head(tree)),0xc),pair))return link_node(tree,out,0,A::right(A::head(tree)),pair,strings,calls);
        }else{
            if(less(pair,at(position.node,0xc))){
                auto previous=position;detail::decrement_tree_iterator<A>(&previous,&invalid);
                if(less(at(previous.node,0xc),pair)){
                    if(A::sentinel(A::right(previous.node)))return link_node(tree,out,0,previous.node,pair,strings,calls);
                    return link_node(tree,out,1,position.node,pair,strings,calls);
                }
            }
            if(less(at(position.node,0xc),pair)){
                Iterator end_iterator{tree,A::head(tree)},next=position;increment(next);
                if(!next.owner||next.owner!=end_iterator.owner)invalid();
                if(next.node==end_iterator.node||less(pair,at(next.node,0xc))){
                    if(A::sentinel(A::right(position.node)))return link_node(tree,out,0,position.node,pair,strings,calls);
                    return link_node(tree,out,1,next.node,pair,strings,calls);
                }
            }
        }
    }
    InsertResult inserted;unique(tree,&inserted,pair,strings,calls);
    put(out,0,inserted.iterator.owner);put(out,4,inserted.iterator.node);return out;
}
struct TemporaryKey {
    void* header;char* captured;NativeStringStorage& strings;
    ~TemporaryKey() noexcept {if(captured)strings.release(captured,read<Word>(header)+1);}
};
} // namespace
std::uint32_t* index_native_profile_counter_005070c0(void* tree,const void* key,
    NativeStringStorage& strings,NativeProfileCollectionCalls& calls){
    Iterator found{tree,detail::lower_bound_tree_node<A>(tree,[key](void* node){return less(at(node,0xc),key);})};
    if(!tree)invalid();
    if(found.node==A::head(tree)||less(key,at(found.node,0xc))){
        alignas(4) Word pair[3];copy_key(pair,key,strings);
        char* const captured=read<char*>(pair,4);put<Word>(pair,8,0);
        const TemporaryKey cleanup{pair,captured,strings};Iterator inserted;
        hint(tree,&inserted,found,pair,strings,calls);found=inserted;
    }
    if(!found.owner)invalid();if(found.node==A::head(found.owner))invalid();
    return static_cast<Word*>(at(found.node,0x14));
}
} // namespace bsp
