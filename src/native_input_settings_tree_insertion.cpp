#include "bsp/native_input_settings_tree_insertion.hpp"
#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <array>
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
using Word=std::uint32_t;
static_assert(sizeof(void*)==4);
enum class Kind { sensitivity, preset, controller, int_string, int_set };
template<Kind K> constexpr Word color_offset=K==Kind::controller?0x20:K==Kind::int_string?0x18:K==Kind::int_set?0x10:0x28;
template<Kind K> using Access=detail::TreeInsertAccess<color_offset<K>,color_offset<K>+1>;
struct Iterator { void* owner;void* node; };
struct InsertResult { void* owner;void* node;unsigned char inserted; };
void* at(void* p,Word n) noexcept { return static_cast<unsigned char*>(p)+n; }
const void* at(const void* p,Word n) noexcept { return static_cast<const unsigned char*>(p)+n; }
Word read(const void* p,Word n=0) noexcept { Word v;std::memcpy(&v,at(p,n),4);return v; }
void write(void* p,Word n,Word v) noexcept { std::memcpy(at(p,n),&v,4); }
void* link(const void* p,Word n=0) noexcept { return reinterpret_cast<void*>(read(p,n)); }
void put(void* p,Word n,const void* v) noexcept { write(p,n,reinterpret_cast<Word>(v)); }
void* allocate(Word n) { return singleton_lifetime_allocate({SingletonAllocationKind::object,n,n}); }
void invalid() { _invalid_parameter_noinfo(); }
struct CompletedMessage {
    NativeLegacySboStringStorage& value;
    ~CompletedMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};
[[noreturn]] void too_long() {
    NativeLegacySboStringStorage text;text.capacity_18=15;text.length_14=0;text.buffer_04.inline_bytes[0]=0;
    native_legacy_sbo_string_assign_counted_00408720(text,"map/set<T> too long",19);
    const CompletedMessage completed{text};throw NativeHardwareLayoutTreeLengthError{text};
}
void copy_string(void* destination,const void* source,NativeStringStorage& strings) {
    const bool same=destination==source;write(destination,0,0);write(destination,4,0);
    if(!same) copy_native_string_header_00be0a30_fragment(destination,strings,source);
}
void vector_copy(void* destination,const void* source) {
    const Word count=read(source,4)?static_cast<Word>(static_cast<std::int32_t>(read(source,8)-read(source,4))>>2):0;
    write(destination,4,0);write(destination,8,0);write(destination,12,0);
    if(!count)return;
    if(count>0x3fffffffu)native_singleton_length_error_00bd0590();
    void* const data=allocate(count*4);put(destination,4,data);put(destination,8,data);write(destination,12,reinterpret_cast<Word>(data)+count*4);
    if(read(source,8)<read(source,4))invalid();
    const Word first=read(source,4),end=read(source,8);if(end<first)invalid();
    std::memmove(data,reinterpret_cast<void*>(first),end-first);write(destination,8,reinterpret_cast<Word>(data)+end-first);
}
void destroy_vector(void* header) noexcept {
    if(link(header,4))singleton_lifetime_free(link(header,4));
    write(header,4,0);write(header,8,0);write(header,12,0);
}
void empty_tree(void* header) {
    void* const head=allocate(0x1c);
    write(head,0,0);write(head,4,0);write(head,8,0);
    static_cast<unsigned char*>(head)[0x18]=1;static_cast<unsigned char*>(head)[0x19]=0;
    put(header,4,head);static_cast<unsigned char*>(head)[0x19]=1;
    put(head,4,head);put(head,0,head);put(head,8,head);write(header,8,0);
}
void copy_empty_controller_default(void* destination,const void* source) {
    // Every caller here copies the freshly constructed empty default from
    // 006A6900. This is the reached 006A2930/006A1C40/006A1290 path, not a
    // general nonempty tree-copy API. Opaque destination word0 stays untouched.
    empty_tree(destination);void* const head=link(destination,4);
    put(head,4,head);write(destination,8,read(source,8));put(head,0,head);put(link(destination,4),8,link(destination,4));
}
void destroy_empty_tree(void* header) noexcept {
    put(link(header,4),4,link(header,4));write(header,8,0);
    put(link(header,4),0,link(header,4));put(link(header,4),8,link(header,4));
    singleton_lifetime_free(link(header,4));write(header,4,0);write(header,8,0);
}
template<Kind K> void destroy_mapped(void* value,NativeStringStorage& strings) noexcept {
    if constexpr(K==Kind::sensitivity)destroy_vector(at(value,4));
    else if constexpr(K==Kind::controller)destroy_empty_tree(value);
    else if constexpr(K==Kind::int_string)destroy_native_string_header_0041dd20(value,strings);
}
template<Kind K> void destroy_pair(void* pair,NativeStringStorage& strings) noexcept {
    if constexpr(K!=Kind::int_set) {
        destroy_mapped<K>(at(pair,K==Kind::int_string?4u:8u),strings);
        if constexpr(K!=Kind::int_string)destroy_native_string_header_0041dd20(pair,strings);
    }
}
template<Kind K> void copy_mapped(void* destination,const void* source,NativeStringStorage& strings) {
    if constexpr(K==Kind::sensitivity) { write(destination,0,read(source));vector_copy(at(destination,4),at(source,4)); }
    else if constexpr(K==Kind::preset)std::memcpy(destination,source,20);
    else if constexpr(K==Kind::controller)copy_empty_controller_default(destination,source);
    else if constexpr(K==Kind::int_string)copy_string(destination,source,strings);
}
template<Kind K> void make_pair(void* destination,const void* key,const void* mapped,NativeStringStorage& strings) {
    if constexpr(K==Kind::int_set)write(destination,0,read(key));
    else if constexpr(K==Kind::int_string) { write(destination,0,read(key));copy_mapped<K>(at(destination,4),mapped,strings); }
    else {
        copy_string(destination,key,strings);
        try { copy_mapped<K>(at(destination,8),mapped,strings); }
        catch(...) { destroy_native_string_header_0041dd20(destination,strings);throw; }
    }
}
template<Kind K> struct MappedGuard { void* p;NativeStringStorage& s;~MappedGuard() noexcept { destroy_mapped<K>(p,s); } };
template<Kind K> struct PairGuard { void* p;NativeStringStorage& s;~PairGuard() noexcept { destroy_pair<K>(p,s); } };
template<Kind K> void* make_node(void* left,void* parent,void* right,const void* pair,std::uint8_t color,NativeStringStorage* strings) {
    void* const node=allocate(color_offset<K>+4);
    try {
        put(node,0,left);put(node,4,parent);put(node,8,right);
        if constexpr(K==Kind::int_set)write(node,12,read(pair));
        else make_pair<K>(at(node,12),pair,at(pair,K==Kind::int_string?4u:8u),*strings);
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
template<Kind K> bool less(const void* left,const void* right) noexcept {
    if constexpr(K==Kind::int_string||K==Kind::int_set)return static_cast<std::int32_t>(read(left))<static_cast<std::int32_t>(read(right));
    else return less_native_string_headers_00443d00(left,right);
}
template<Kind K> void increment(Iterator& iterator) {
    using A=Access<K>;void* node=iterator.node;if(A::sentinel(node))invalid();
    if(!A::sentinel(A::right(node))) { node=A::right(node);while(!A::sentinel(A::left(node)))node=A::left(node); }
    else { void* parent=A::parent(node);while(!A::sentinel(parent)&&node==A::right(parent)){node=parent;parent=A::parent(parent);}node=parent; }
    iterator.node=node;
}
template<Kind K> Iterator* link_node(void* tree,Iterator* output,std::uint8_t left,void* parent,const void* pair,NativeStringStorage* strings) {
    constexpr Word limit=K==Kind::int_set?0x3ffffffeu:K==Kind::int_string?0x15555554u:K==Kind::controller?0x0ccccccbu:0x09249248u;
    detail::link_tree_node<Access<K>>(tree,output,left,parent,pair,limit,
        [strings](void* l,void* p,void* r,const void* value,std::uint8_t c){return make_node<K>(l,p,r,value,c,strings);},
        &detail::rotate_left_inlined<Access<K>>,&rotate_right<K>,&too_long);
    return output;
}
void publish(InsertResult* output,void* owner,void* node,std::uint8_t inserted) noexcept {
    put(output,0,owner);put(output,4,node);static_cast<unsigned char*>(static_cast<void*>(output))[8]=inserted;
}
template<Kind K> InsertResult* unique(void* tree,InsertResult* output,const void* pair,NativeStringStorage* strings) {
    return detail::insert_unique_tree_pair<Access<K>,Iterator>(tree,output,pair,
        [pair](void* node){return less<K>(pair,at(node,12));},
        [pair](void* node){return less<K>(at(node,12),pair);},
        [](Iterator& i){detail::decrement_tree_iterator<Access<K>>(&i,&invalid);},
        [strings](void* t,Iterator* o,std::uint8_t l,void* p,const void* v){return link_node<K>(t,o,l,p,v,strings);},&publish);
}
template<Kind K> Iterator* hint(void* tree,Iterator* output,Iterator position,const void* pair,NativeStringStorage& strings) {
    using A=Access<K>;
    if(A::count(tree)==0)return link_node<K>(tree,output,1,A::head(tree),pair,&strings);
    void* const minimum=A::left(A::head(tree));if(!position.owner||position.owner!=tree)invalid();
    if(position.node==minimum) {
        if(less<K>(pair,at(position.node,12)))return link_node<K>(tree,output,1,position.node,pair,&strings);
    } else {
        void* const end=A::head(tree);if(!position.owner||position.owner!=tree)invalid();
        if(position.node==end) {
            if(less<K>(at(A::right(A::head(tree)),12),pair))return link_node<K>(tree,output,0,A::right(A::head(tree)),pair,&strings);
        } else {
            if(less<K>(pair,at(position.node,12))) {
                auto previous=position;detail::decrement_tree_iterator<A>(&previous,&invalid);
                if(less<K>(at(previous.node,12),pair)) {
                    if(A::sentinel(A::right(previous.node)))return link_node<K>(tree,output,0,previous.node,pair,&strings);
                    return link_node<K>(tree,output,1,position.node,pair,&strings);
                }
            }
            if(less<K>(at(position.node,12),pair)) {
                Iterator end_iterator{tree,A::head(tree)};auto next=position;increment<K>(next);
                if(!next.owner||next.owner!=end_iterator.owner)invalid();
                if(next.node==end_iterator.node||less<K>(pair,at(next.node,12))) {
                    if(A::sentinel(A::right(position.node)))return link_node<K>(tree,output,0,position.node,pair,&strings);
                    return link_node<K>(tree,output,1,next.node,pair,&strings);
                }
            }
        }
    }
    InsertResult inserted;unique<K>(tree,&inserted,pair,&strings);put(output,0,inserted.owner);put(output,4,inserted.node);return output;
}
template<Kind K> void* index(void* tree,const void* key,Word preimage,NativeStringStorage& strings) {
    using A=Access<K>;
    Iterator found{tree,detail::lower_bound_tree_node<A>(tree,[key](void* node){return less<K>(at(node,12),key);})};
    if constexpr(K!=Kind::int_string)if(!tree)invalid();
    if(found.node==A::head(tree)||less<K>(key,at(found.node,12))) {
        alignas(4) unsigned char mapped[20];
        if constexpr(K==Kind::sensitivity) { write(mapped,0,preimage);write(mapped,8,0);write(mapped,12,0);write(mapped,16,0); }
        else if constexpr(K==Kind::preset) { write(mapped,0,0xffffffffu);write(mapped,4,0);write(mapped,8,0);write(mapped,12,0);write(mapped,16,preimage&0xffffff00u); }
        else if constexpr(K==Kind::controller)empty_tree(mapped);
        else { write(mapped,0,0);write(mapped,4,0); }
        const MappedGuard<K> mapped_guard{mapped,strings};
        alignas(4) unsigned char pair[28];make_pair<K>(pair,key,mapped,strings);
        const PairGuard<K> pair_guard{pair,strings};Iterator inserted;
        hint<K>(tree,&inserted,found,pair,strings);found=inserted;
    }
    if(!found.owner)invalid();if(found.node==link(found.owner,4))invalid();
    return at(found.node,K==Kind::int_string?16u:20u);
}
} // namespace
void* index_native_input_sensitivity_tree_0055a9a0(void* t,const NativeString* k,Word pre,NativeStringStorage& s){return index<Kind::sensitivity>(t,k,pre,s);}
void* index_native_input_preset_tree_006a1e70(void* t,const NativeString* k,Word pre,NativeStringStorage& s){return index<Kind::preset>(t,k,pre,s);}
void* index_native_input_controller_tree_006a6900(void* t,const NativeString* k,NativeStringStorage& s){return index<Kind::controller>(t,k,0,s);}
void* index_native_input_controller_name_006a1f80(void* t,const std::int32_t* k,NativeStringStorage& s){return index<Kind::int_string>(t,k,0,s);}
void* insert_native_input_integer_key_0069fa40(void* t,void* o,const std::int32_t* k){return unique<Kind::int_set>(t,static_cast<InsertResult*>(o),k,nullptr);}
} // namespace bsp
