#include "bsp/native_input_device_index.hpp"
#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_input_settings_tree_cleanup.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <array>
#include <cstdint>
#include <cstring>
#include <cstdlib>

namespace bsp { namespace {
using Word=std::uint32_t;
using Access=detail::TreeInsertAccess<0x98,0x99>;
static_assert(sizeof(void*)==4);
struct Iterator { void* owner; void* node; };
struct InsertResult { void* owner; void* node; unsigned char inserted; };
void* at(void* p,Word n) noexcept {return static_cast<unsigned char*>(p)+n;}
const void* at(const void* p,Word n) noexcept {return static_cast<const unsigned char*>(p)+n;}
Word read(const void* p,Word n=0) noexcept {Word v;std::memcpy(&v,at(p,n),4);return v;}
void write(void* p,Word n,Word v) noexcept {std::memcpy(at(p,n),&v,4);}
void* link(const void* p,Word n=0) noexcept {return reinterpret_cast<void*>(read(p,n));}
void put(void* p,Word n,const void* v) noexcept {write(p,n,reinterpret_cast<Word>(v));}
void* allocate(Word n) {return singleton_lifetime_allocate({SingletonAllocationKind::object,n,n});}
void invalid() {_invalid_parameter_noinfo();}
struct MessageGuard {NativeLegacySboStringStorage& value;~MessageGuard() noexcept {native_legacy_sbo_string_destroy_004072d0(value);}};
[[noreturn]] void too_long() {
    NativeLegacySboStringStorage text;text.capacity_18=15;text.length_14=0;text.buffer_04.inline_bytes[0]=0;
    native_legacy_sbo_string_assign_counted_00408720(text,"map/set<T> too long",19);
    const MessageGuard guard{text};throw NativeHardwareLayoutTreeLengthError{text};
}
bool less(const void* a,const void* b) noexcept {
    const Word al=read(a),bl=read(b);
    if(!al)return bl!=0;
    if(!bl)return false;
    return _stricmp(static_cast<const char*>(link(a,4)),static_cast<const char*>(link(b,4)))<0;
}
constexpr Word offsets[]={0,0x0c,0x18,0x24,0x50,0x6c,0x78};
constexpr Word sizes[]={0x28,0x28,0x2c,0x1c,0x20,0x2c,0x14};
constexpr Word nils[]={0x25,0x25,0x29,0x19,0x1d,0x29,0x11};
void empty_head(void* field,Word size,Word nil) {
    void* const h=allocate(size);
    put(h,0,nullptr);put(h,4,nullptr);put(h,8,nullptr);
    static_cast<unsigned char*>(h)[nil-1]=1;static_cast<unsigned char*>(h)[nil]=0;
    put(field,4,h);static_cast<unsigned char*>(h)[nil]=1;
    put(h,4,h);put(h,0,h);put(h,8,h);write(field,8,0);
}
void fresh_device(void* target,NativeStringStorage& strings) {
    unsigned made=0;
    try {
        for(unsigned i=0;i<4;++i){empty_head(at(target,offsets[i]),sizes[i],nils[i]);++made;}
        for(Word o:{0x30u,0x40u}){write(target,o+4,0);write(target,o+8,0);write(target,o+12,0);}
        empty_head(at(target,offsets[4]),sizes[4],nils[4]);++made;
        for(Word o:{0x5cu}){write(target,o+4,0);write(target,o+8,0);write(target,o+12,0);}
        for(unsigned i=5;i<7;++i){empty_head(at(target,offsets[i]),sizes[i],nils[i]);++made;}
    } catch(...) {while(made){const unsigned i=--made;singleton_lifetime_free(link(target,offsets[i]+4));put(target,offsets[i]+4,nullptr);}throw;}
    (void)strings;
}
void copy_empty_device(void* target,const void* source,NativeStringStorage& strings) {
    // The reached source is the just-constructed empty 0055B520 record.
    // Native copy constructors leave destination opaque bytes untouched.
    // The reached source has empty nested containers, so create distinct heads.
    fresh_device(target,strings);
    (void)source;
}
void destroy_device(void* p,NativeStringStorage& strings) noexcept {destroy_native_input_device_record_0055b690(p,strings);}
struct DeviceGuard {void* p;NativeStringStorage& s;~DeviceGuard() noexcept {destroy_device(p,s);}};
struct PairGuard {void* p;NativeStringStorage& s;~PairGuard() noexcept {destroy_device(at(p,8),s);destroy_native_string_header_0041dd20(p,s);}};
void copy_pair(void* dst,const void* key,const void* mapped,NativeStringStorage& strings) {
    write(dst,0,0);write(dst,4,0);copy_native_string_header_00be0a30_fragment(dst,strings,key);
    try {copy_empty_device(at(dst,8),mapped,strings);}
    catch(...) {destroy_native_string_header_0041dd20(dst,strings);throw;}
}
void* make_node(void* left,void* parent,void* right,const void* pair,std::uint8_t color,NativeStringStorage& strings) {
    void* const node=allocate(0x9c);
    try {put(node,0,left);put(node,4,parent);put(node,8,right);copy_pair(at(node,12),pair,at(pair,8),strings);
        static_cast<unsigned char*>(node)[0x98]=color;static_cast<unsigned char*>(node)[0x99]=0;
    } catch(...) {singleton_lifetime_free(node);throw;}
    return node;
}
void rotate_right(void* tree,void* node) noexcept {
    void* const pivot=Access::left(node);Access::left(node)=Access::right(pivot);
    if(!Access::sentinel(Access::right(pivot)))Access::parent(Access::right(pivot))=node;
    Access::parent(pivot)=Access::parent(node);
    if(node==Access::parent(Access::head(tree)))Access::parent(Access::head(tree))=pivot;
    else if(node==Access::right(Access::parent(node)))Access::right(Access::parent(node))=pivot;
    else Access::left(Access::parent(node))=pivot;
    Access::right(pivot)=node;Access::parent(node)=pivot;
}
Iterator* link_node(void* tree,Iterator* out,std::uint8_t left,void* parent,const void* pair,NativeStringStorage& strings) {
    detail::link_tree_node<Access>(tree,out,left,parent,pair,0x01d41d40u,
        [&strings](void* l,void* p,void* r,const void* v,std::uint8_t c){return make_node(l,p,r,v,c,strings);},
        &detail::rotate_left_inlined<Access>,&rotate_right,&too_long);return out;
}
void increment(Iterator& i) {
    void* n=i.node;if(Access::sentinel(n))invalid();
    if(!Access::sentinel(Access::right(n))){n=Access::right(n);while(!Access::sentinel(Access::left(n)))n=Access::left(n);}
    else {void* p=Access::parent(n);while(!Access::sentinel(p)&&n==Access::right(p)){n=p;p=Access::parent(p);}n=p;}i.node=n;
}
void publish(InsertResult* out,void* owner,void* node,std::uint8_t inserted) noexcept {
    put(out,0,owner);put(out,4,node);static_cast<unsigned char*>(static_cast<void*>(out))[8]=inserted;
}
InsertResult* unique(void* tree,InsertResult* out,const void* pair,NativeStringStorage& strings) {
    return detail::insert_unique_tree_pair<Access,Iterator>(tree,out,pair,
        [pair](void* n){return less(pair,at(n,12));},[pair](void* n){return less(at(n,12),pair);},
        [](Iterator& i){detail::decrement_tree_iterator<Access>(&i,&invalid);},
        [&strings](void* t,Iterator* o,std::uint8_t l,void* p,const void* v){return link_node(t,o,l,p,v,strings);},&publish);
}
Iterator* hint(void* tree,Iterator* out,Iterator pos,const void* pair,NativeStringStorage& strings) {
    if(Access::count(tree)==0)return link_node(tree,out,1,Access::head(tree),pair,strings);
    void* const minimum=Access::left(Access::head(tree));if(!pos.owner||pos.owner!=tree)invalid();
    if(pos.node==minimum){if(less(pair,at(pos.node,12)))return link_node(tree,out,1,pos.node,pair,strings);}
    else {void* const end=Access::head(tree);if(!pos.owner||pos.owner!=tree)invalid();
        if(pos.node==end){if(less(at(Access::right(end),12),pair))return link_node(tree,out,0,Access::right(end),pair,strings);}
        else {if(less(pair,at(pos.node,12))){auto prev=pos;detail::decrement_tree_iterator<Access>(&prev,&invalid);
                if(less(at(prev.node,12),pair)){if(Access::sentinel(Access::right(prev.node)))return link_node(tree,out,0,prev.node,pair,strings);
                    return link_node(tree,out,1,pos.node,pair,strings);}}
            if(less(at(pos.node,12),pair)){Iterator next=pos;increment(next);
                if(!next.owner||next.owner!=tree)invalid();
                if(next.node==end||less(pair,at(next.node,12))){if(Access::sentinel(Access::right(pos.node)))return link_node(tree,out,0,pos.node,pair,strings);
                    return link_node(tree,out,1,next.node,pair,strings);}}}}
    InsertResult inserted;unique(tree,&inserted,pair,strings);put(out,0,inserted.owner);put(out,4,inserted.node);return out;
}
} // namespace
void* index_native_input_device_tree_0055c110(void* tree,const NativeString* key,NativeStringStorage& strings) {
    Iterator found{tree,detail::lower_bound_tree_node<Access>(tree,[key](void* n){return less(at(n,12),key);})};
    if(!tree)invalid();
    if(found.node==Access::head(tree)||less(key,at(found.node,12))) {
        alignas(4) unsigned char mapped[0x84];fresh_device(mapped,strings);DeviceGuard mapped_guard{mapped,strings};
        alignas(4) unsigned char pair[0x8c];copy_pair(pair,key,mapped,strings);PairGuard pair_guard{pair,strings};
        Iterator inserted;hint(tree,&inserted,found,pair,strings);found=inserted;
    }
    if(!found.owner||found.node==link(found.owner,4))invalid();return at(found.node,0x14);
}
} // namespace bsp
