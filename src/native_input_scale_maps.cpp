#include "bsp/native_input_scale_maps.hpp"
#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_legacy_sbo_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string.h>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using W = std::uint32_t;
using Outer = detail::TreeInsertAccess<0x1c,0x1d>;
using Inner = detail::TreeInsertAccess<0x14,0x15>;
using Strings = detail::TreeInsertAccess<0x18,0x19>;
void* at(const void* p, W n) noexcept { return static_cast<unsigned char*>(const_cast<void*>(p))+n; }
W read(const void* p,W n=0) noexcept { W v;std::memcpy(&v,at(p,n),4);return v; }
void write(void* p,W n,W v) noexcept { std::memcpy(at(p,n),&v,4); }
void* ptr(const void* p,W n=0) noexcept { return reinterpret_cast<void*>(read(p,n)); }
void put(void* p,W n,const void* value) noexcept { write(p,n,reinterpret_cast<W>(value)); }
void* alloc(W bytes) { return singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes}); }
void* head(void* tree,W bytes,W nil) {
    auto* h=alloc(bytes);put(h,0,nullptr);put(h,4,nullptr);put(h,8,nullptr);
    static_cast<unsigned char*>(h)[nil-1]=1;static_cast<unsigned char*>(h)[nil]=1;
    put(h,0,h);put(h,4,h);put(h,8,h);put(tree,4,h);write(tree,8,0);return h;
}
void free_inner_nodes(void* node) noexcept {
    while(!static_cast<unsigned char*>(node)[0x15]) {
        free_inner_nodes(ptr(node,8));auto* left=ptr(node);
        singleton_lifetime_free(node);node=left;
    }
}
void destroy_inner(void* tree) noexcept {
    auto* h=ptr(tree,4);if(!h)return;
    free_inner_nodes(ptr(h,4));singleton_lifetime_free(h);put(tree,4,nullptr);write(tree,8,0);
}
void free_outer_nodes(void* node) noexcept {
    while(!static_cast<unsigned char*>(node)[0x1d]) {
        free_outer_nodes(ptr(node,8));auto* left=ptr(node);
        destroy_inner(at(node,0x10));singleton_lifetime_free(node);node=left;
    }
}
void init_inner(void* tree) { head(tree,0x18,0x15); }
void* copy_inner_node(const void* src,void* parent,void* sentinel) {
    if(static_cast<unsigned char*>(const_cast<void*>(src))[0x15])return sentinel;
    auto* n=alloc(0x18);
    put(n,4,parent);put(n,0,sentinel);put(n,8,sentinel);
    write(n,0x0c,read(src,0x0c));write(n,0x10,read(src,0x10));
    static_cast<unsigned char*>(n)[0x14]=static_cast<unsigned char*>(const_cast<void*>(src))[0x14];
    static_cast<unsigned char*>(n)[0x15]=0;
    try { put(n,0,copy_inner_node(ptr(src),n,sentinel));
          put(n,8,copy_inner_node(ptr(src,8),n,sentinel)); }
    catch(...) { free_inner_nodes(ptr(n));free_inner_nodes(ptr(n,8));singleton_lifetime_free(n);throw; }
    return n;
}
void copy_inner(void* dst,const void* src) {
    auto* h=head(dst,0x18,0x15);
    try { put(h,4,copy_inner_node(ptr(ptr(src,4),4),h,h));write(dst,8,read(src,8));
          auto* n=ptr(h,4);if(n!=h) {auto* l=n;while(ptr(l)!=h)l=ptr(l);put(h,0,l);
              auto* r=n;while(ptr(r,8)!=h)r=ptr(r,8);put(h,8,r);} }
    catch(...) {destroy_inner(dst);throw;}
}
void* copy_outer_node(const void* src,void* parent,void* sentinel) {
    if(static_cast<unsigned char*>(const_cast<void*>(src))[0x1d])return sentinel;
    auto* n=alloc(0x20);
    put(n,4,parent);put(n,0,sentinel);put(n,8,sentinel);write(n,0x0c,read(src,0x0c));
    put(n,0x14,nullptr);write(n,0x18,0);
    static_cast<unsigned char*>(n)[0x1c]=static_cast<unsigned char*>(const_cast<void*>(src))[0x1c];
    static_cast<unsigned char*>(n)[0x1d]=0;
    try { copy_inner(at(n,0x10),at(src,0x10));
          put(n,0,copy_outer_node(ptr(src),n,sentinel));
          put(n,8,copy_outer_node(ptr(src,8),n,sentinel)); }
    catch(...) { free_outer_nodes(ptr(n));free_outer_nodes(ptr(n,8));destroy_inner(at(n,0x10));singleton_lifetime_free(n);throw; }
    return n;
}
int ci(const void* a,const void* b) {
    const auto al=read(a),bl=read(b);
    if(!al)return bl ? -1:0;if(!bl)return 1;
    return _stricmp(static_cast<const char*>(ptr(a,4)),static_cast<const char*>(ptr(b,4)));
}
template<class Access> void rotate_right_t(void* tree,void* node) noexcept {
    auto* y=Access::left(node);Access::left(node)=Access::right(y);
    if(!Access::sentinel(Access::right(y)))Access::parent(Access::right(y))=node;
    Access::parent(y)=Access::parent(node);auto* h=Access::head(tree);
    if(node==Access::parent(h))Access::parent(h)=y;
    else if(node==Access::right(Access::parent(node)))Access::right(Access::parent(node))=y;
    else Access::left(Access::parent(node))=y;
    Access::right(y)=node;Access::parent(node)=y;
}
template<class Access,class Create> void* insert_leaf(void* tree,void* parent,bool left,
    const void* pair,W limit,Create create) {
    void* output[2]{};
    detail::link_tree_node<Access>(tree,output,left?1:0,parent,pair,limit,
      create,
      [](void* t,void* n){detail::rotate_left_inlined<Access>(t,n);},
      [](void* t,void* n){rotate_right_t<Access>(t,n);},
      []{ NativeLegacySboStringStorage text;
          text.capacity_18=15;text.length_14=0;text.buffer_04.inline_bytes[0]=0;
          native_legacy_sbo_string_assign_counted_00408720(text,"map/set<T> too long",19);
          struct Cleanup { NativeLegacySboStringStorage& t;
              ~Cleanup() noexcept {native_legacy_sbo_string_destroy_004072d0(t);} } cleanup{text};
          throw NativeHardwareLayoutTreeLengthError{text};});
    return output[1];
}
void* new_outer(void* h,void* parent,void* right,const void* pair,std::uint8_t color) {
    auto* n=alloc(0x20);put(n,0,h);put(n,4,parent);put(n,8,right);
    write(n,0x0c,read(pair));static_cast<unsigned char*>(n)[0x1c]=color;
    static_cast<unsigned char*>(n)[0x1d]=0;
    put(n,0x14,nullptr);write(n,0x18,0);
    try {copy_inner(at(n,0x10),at(pair,4));}
    catch(...) {singleton_lifetime_free(n);throw;}
    return n;
}
void* new_string(void* h,void* parent,void* right,const void* pair,
    std::uint8_t color,NativeStringStorage& storage) {
    auto* n=alloc(0x1c);put(n,0,h);put(n,4,parent);put(n,8,right);
    write(n,0x0c,0);write(n,0x10,0);
    try {copy_native_string_header_00be0a30_fragment(at(n,0x0c),storage,pair);}
    catch(...) {singleton_lifetime_free(n);throw;}
    write(n,0x14,read(pair,8));static_cast<unsigned char*>(n)[0x18]=color;
    static_cast<unsigned char*>(n)[0x19]=0;return n;
}
} // namespace

void* copy_native_input_scale_map_0055b400(void* dst,const void* src) {
    auto* h=head(dst,0x20,0x1d);
    try {put(h,4,copy_outer_node(ptr(ptr(src,4),4),h,h));write(dst,8,read(src,8));
         auto* n=ptr(h,4);if(n!=h){auto* l=n;while(ptr(l)!=h)l=ptr(l);put(h,0,l);
             auto* r=n;while(ptr(r,8)!=h)r=ptr(r,8);put(h,8,r);}}
    catch(...) {destroy_native_input_scale_map_0055b490(dst);throw;}
    return dst;
}
void destroy_native_input_scale_map_0055b490(void* map) noexcept {
    auto* h=ptr(map,4);if(!h)return;
    free_outer_nodes(ptr(h,4));singleton_lifetime_free(h);
}
void* lookup_or_insert_native_input_scalar_tree_006a5aa0(void* map,const std::int32_t* key) {
    auto* h=ptr(map,4);auto* candidate=h;auto* node=ptr(h,4);auto* parent=h;bool left=true;
    while(!Outer::sentinel(node)) { parent=node;left=static_cast<std::int32_t>(read(node,0x0c))>=*key;
        if(left)candidate=node;node=ptr(node,left?0:8); }
    if(candidate!=h && static_cast<std::int32_t>(read(candidate,0x0c))<=*key)return at(candidate,0x10);
    W default_tree[3];init_inner(default_tree);
    W pair[4];pair[0]=static_cast<W>(*key);put(pair,8,nullptr);write(pair,12,0);
    try {copy_inner(pair+1,default_tree);
         try {auto* result=insert_leaf<Outer>(map,parent,left,pair,0x0ffffffe,new_outer);
              destroy_inner(pair+1);destroy_inner(default_tree);return at(result,0x10);}
         catch(...) {destroy_inner(pair+1);throw;} }
    catch(...) {destroy_inner(default_tree);throw;}
}
float* lookup_or_insert_native_input_float_00444be0(void* map,const void* name,
    NativeStringStorage& storage) {
    auto* h=ptr(map,4);auto* candidate=h;auto* node=ptr(h,4);auto* parent=h;bool left=true;
    while(!Strings::sentinel(node)) { parent=node;left=ci(at(node,0x0c),name)>=0;
        if(left)candidate=node;node=ptr(node,left?0:8); }
    if(candidate!=h && ci(name,at(candidate,0x0c))>=0)
        return static_cast<float*>(at(candidate,0x14));
    unsigned char pair[12];write(pair,0,0);write(pair,4,0);write(pair,8,0);
    try {copy_native_string_header_00be0a30_fragment(pair,storage,name);
        auto* result=insert_leaf<Strings>(map,parent,left,pair,0x15555554,
            [&storage](void* a,void* b,void* c,const void* d,std::uint8_t e){return new_string(a,b,c,d,e,storage);});
        destroy_native_string_header_0041dd20(pair,storage);
        return static_cast<float*>(at(result,0x14));
    } catch(...) {destroy_native_string_header_0041dd20(pair,storage);throw;}
}
} // namespace bsp
