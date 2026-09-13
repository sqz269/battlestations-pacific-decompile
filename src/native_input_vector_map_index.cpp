#include "bsp/native_input_vector_map_index.hpp"
#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
enum class Kind { words, descriptors, bits };
template<Kind K> constexpr Word color_offset = K == Kind::bits ? 0x28 : 0x24;
template<Kind K> using Access = detail::TreeInsertAccess<color_offset<K>, color_offset<K> + 1>;
struct Iterator { void* owner; void* node; };
struct InsertResult { void* owner; void* node; unsigned char inserted; };
void* at(void* p, Word n) noexcept { return static_cast<unsigned char*>(p) + n; }
const void* at(const void* p, Word n) noexcept { return static_cast<const unsigned char*>(p) + n; }
Word read(const void* p, Word n = 0) noexcept { Word v; std::memcpy(&v, at(p,n),4); return v; }
void write(void* p, Word n, Word v) noexcept { std::memcpy(at(p,n),&v,4); }
void* link(const void* p, Word n = 0) noexcept { return reinterpret_cast<void*>(read(p,n)); }
void put(void* p, Word n, const void* v) noexcept { write(p,n,reinterpret_cast<Word>(v)); }
void* allocate(Word n) { return singleton_lifetime_allocate({SingletonAllocationKind::object,n,n}); }
void invalid() { _invalid_parameter_noinfo(); }
struct CompletedMessage { NativeLegacySboStringStorage& value; ~CompletedMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(value); } };
[[noreturn]] void too_long() {
    NativeLegacySboStringStorage text; text.capacity_18=15; text.length_14=0; text.buffer_04.inline_bytes[0]=0;
    native_legacy_sbo_string_assign_counted_00408720(text,"map/set<T> too long",19);
    const CompletedMessage completed{text}; throw NativeHardwareLayoutTreeLengthError{text};
}
void copy_string(void* dst,const void* src,NativeStringStorage& strings) {
    const bool same=dst==src; write(dst,0,0); write(dst,4,0);
    if(!same) copy_native_string_header_00be0a30_fragment(dst,strings,src);
}
template<Kind K> constexpr Word mapped_width = K == Kind::bits ? 20 : 16;
template<Kind K> void copy_mapped(void* dst,const void* src) {
    if constexpr(K==Kind::bits) write(dst,0,read(src));
    const Word width=K==Kind::descriptors?20:4;
    const Word base=K==Kind::bits?8:4;
    const Word first=read(src,base);
    const Word count=first?static_cast<Word>(static_cast<std::int32_t>(read(src,base+4)-first)/static_cast<std::int32_t>(width)):0;
    write(dst,base,0); write(dst,base+4,0); write(dst,base+8,0);
    if(!count)return;
    if(count>0xffffffffu/width)native_singleton_length_error_00bd0590();
    void* const data=allocate(count*width); put(dst,base,data); put(dst,base+4,data); write(dst,base+8,reinterpret_cast<Word>(data)+count*width);
    if(read(src,base+4)<first)invalid();
    std::memmove(data,reinterpret_cast<void*>(first),read(src,base+4)-first);
    write(dst,base+4,reinterpret_cast<Word>(data)+read(src,base+4)-first);
}
template<Kind K> void destroy_mapped(void* value) noexcept {
    const Word base=K==Kind::bits?8:4;
    if(link(value,base))singleton_lifetime_free(link(value,base));
    write(value,base,0);write(value,base+4,0);write(value,base+8,0);
}
template<Kind K> void destroy_pair(void* pair,NativeStringStorage& strings) noexcept {
    destroy_mapped<K>(at(pair,8));destroy_native_string_header_0041dd20(pair,strings);
}
template<Kind K> void make_pair(void* dst,const void* key,const void* mapped,NativeStringStorage& strings) {
    copy_string(dst,key,strings);
    try { copy_mapped<K>(at(dst,8),mapped); }
    catch(...) { destroy_native_string_header_0041dd20(dst,strings); throw; }
}
template<Kind K> struct MappedGuard { void* value; ~MappedGuard() noexcept { destroy_mapped<K>(value); } };
template<Kind K> struct PairGuard { void* value; NativeStringStorage& strings; ~PairGuard() noexcept { destroy_pair<K>(value,strings); } };
template<Kind K> void* make_node(void* left,void* parent,void* right,const void* pair,std::uint8_t color,NativeStringStorage* strings) {
    void* const node=allocate(color_offset<K>+4);
    try {
        put(node,0,left);put(node,4,parent);put(node,8,right);
        make_pair<K>(at(node,12),pair,at(pair,8),*strings);
        static_cast<unsigned char*>(node)[color_offset<K>]=color;
        static_cast<unsigned char*>(node)[color_offset<K>+1]=0;
    } catch(...) { singleton_lifetime_free(node);throw; }
    return node;
}
template<Kind K> void rotate_right(void* tree,void* node) noexcept {
    using A=Access<K>;void* const pivot=A::left(node);A::left(node)=A::right(pivot);
    if(!A::sentinel(A::right(pivot)))A::parent(A::right(pivot))=node;
    A::parent(pivot)=A::parent(node);
    if(node==A::parent(A::head(tree)))A::parent(A::head(tree))=pivot;
    else if(node==A::right(A::parent(node)))A::right(A::parent(node))=pivot;
    else A::left(A::parent(node))=pivot;
    A::right(pivot)=node;A::parent(node)=pivot;
}
bool less(const void* a,const void* b) noexcept { return less_native_string_headers_00443d00(a,b); }
template<Kind K> void increment(Iterator& i) {
    using A=Access<K>;void* node=i.node;if(A::sentinel(node))invalid();
    if(!A::sentinel(A::right(node))) {node=A::right(node);while(!A::sentinel(A::left(node)))node=A::left(node);}
    else {void* parent=A::parent(node);while(!A::sentinel(parent)&&node==A::right(parent)){node=parent;parent=A::parent(node);}node=parent;}
    i.node=node;
}
template<Kind K> Iterator* link_node(void* tree,Iterator* output,std::uint8_t left,void* parent,const void* pair,NativeStringStorage* strings) {
    constexpr Word limit=K==Kind::bits?0x09249248u:0x0aaaaaa8u;
    detail::link_tree_node<Access<K>>(tree,output,left,parent,pair,limit,
        [strings](void* l,void* p,void* r,const void* v,std::uint8_t c){return make_node<K>(l,p,r,v,c,strings);},
        &detail::rotate_left_inlined<Access<K>>,&rotate_right<K>,&too_long);
    return output;
}
void publish(InsertResult* out,void* owner,void* node,std::uint8_t inserted) noexcept {
    put(out,0,owner);put(out,4,node);static_cast<unsigned char*>(static_cast<void*>(out))[8]=inserted;
}
template<Kind K> InsertResult* unique(void* tree,InsertResult* out,const void* pair,NativeStringStorage* strings) {
    return detail::insert_unique_tree_pair<Access<K>,Iterator>(tree,out,pair,
        [pair](void* node){return less(pair,at(node,12));},
        [pair](void* node){return less(at(node,12),pair);},
        [](Iterator& i){detail::decrement_tree_iterator<Access<K>>(&i,&invalid);},
        [strings](void* t,Iterator* o,std::uint8_t l,void* p,const void* v){return link_node<K>(t,o,l,p,v,strings);},&publish);
}
template<Kind K> Iterator* hint(void* tree,Iterator* out,Iterator position,const void* pair,NativeStringStorage& strings) {
    using A=Access<K>;
    if(A::count(tree)==0)return link_node<K>(tree,out,1,A::head(tree),pair,&strings);
    void* const minimum=A::left(A::head(tree));if(!position.owner||position.owner!=tree)invalid();
    if(position.node==minimum) {
        if(less(pair,at(position.node,12)))return link_node<K>(tree,out,1,position.node,pair,&strings);
    } else {
        void* const end=A::head(tree);if(!position.owner||position.owner!=tree)invalid();
        if(position.node==end) {
            if(less(at(A::right(end),12),pair))return link_node<K>(tree,out,0,A::right(end),pair,&strings);
        } else {
            if(less(pair,at(position.node,12))) {
                auto previous=position;detail::decrement_tree_iterator<A>(&previous,&invalid);
                if(less(at(previous.node,12),pair)) {
                    if(A::sentinel(A::right(previous.node)))return link_node<K>(tree,out,0,previous.node,pair,&strings);
                    return link_node<K>(tree,out,1,position.node,pair,&strings);
                }
            }
            if(less(at(position.node,12),pair)) {
                Iterator end_iterator{tree,A::head(tree)};auto next=position;increment<K>(next);
                if(!next.owner||next.owner!=end_iterator.owner)invalid();
                if(next.node==end_iterator.node||less(pair,at(next.node,12))) {
                    if(A::sentinel(A::right(position.node)))return link_node<K>(tree,out,0,position.node,pair,&strings);
                    return link_node<K>(tree,out,1,next.node,pair,&strings);
                }
            }
        }
    }
    InsertResult inserted;unique<K>(tree,&inserted,pair,&strings);put(out,0,inserted.owner);put(out,4,inserted.node);return out;
}
template<Kind K> void* index(void* tree,const NativeString* key,NativeStringStorage& strings) {
    using A=Access<K>;
    Iterator found{tree,detail::lower_bound_tree_node<A>(tree,[key](void* node){return less(at(node,12),key);})};
    if(!tree)invalid();
    if(found.node==A::head(tree)||less(key,at(found.node,12))) {
        alignas(4) unsigned char mapped[mapped_width<K>];
        if constexpr(K==Kind::bits) write(mapped,0,0);
        constexpr Word base=K==Kind::bits?8:4;
        write(mapped,base,0);write(mapped,base+4,0);write(mapped,base+8,0);
        const MappedGuard<K> mapped_guard{mapped};
        alignas(4) unsigned char pair[8+mapped_width<K>];make_pair<K>(pair,key,mapped,strings);
        const PairGuard<K> pair_guard{pair,strings};Iterator inserted;
        hint<K>(tree,&inserted,found,pair,strings);found=inserted;
    }
    if(!found.owner)invalid();if(found.node==link(found.owner,4))invalid();
    return at(found.node,20);
}
} // namespace
void* index_native_input_word_vector_006a44b0(void* t,const NativeString* k,NativeStringStorage& s){return index<Kind::words>(t,k,s);}
void* index_native_input_descriptor_vector_006a45c0(void* t,const NativeString* k,NativeStringStorage& s){return index<Kind::descriptors>(t,k,s);}
void* index_native_input_bit_vector_006a4ca0(void* t,const NativeString* k,NativeStringStorage& s){return index<Kind::bits>(t,k,s);}
} // namespace bsp
