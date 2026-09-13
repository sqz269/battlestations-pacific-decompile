#include "bsp/native_input_settings_tree_cleanup.hpp"
#include "bsp/global_config.hpp"
#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
using Word=std::uint32_t;
static_assert(sizeof(void*)==4);
enum class Kind { device, controller, defaults, preset, code_vector, sensitivity,
    bits, scalar_string_key, int_set, int_only, int_string };
void* at(void* p, Word n) noexcept { return static_cast<unsigned char*>(p)+n; }
Word read(const void* p, Word n=0) noexcept { Word v;std::memcpy(&v,static_cast<const unsigned char*>(p)+n,4);return v; }
void write(void* p,Word n,Word v) noexcept { std::memcpy(at(p,n),&v,4); }
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
void* link(const void* p,Word n=0) noexcept { return pointer(read(p,n)); }
void put(void* p,Word n,const void* v) noexcept { write(p,n,address(v)); }
Word nil_offset(Kind kind) noexcept {
    switch(kind) {
    case Kind::device:return 0x99;
    case Kind::controller:return 0x21;
    case Kind::defaults:return 0x15;
    case Kind::preset:case Kind::sensitivity:case Kind::bits:return 0x29;
    case Kind::code_vector:return 0x25;
    case Kind::scalar_string_key:case Kind::int_string:return 0x19;
    case Kind::int_set:return 0x1d;
    default:return 0x11;
    }
}
bool nil(Kind kind,const void* p) noexcept { return static_cast<const unsigned char*>(p)[nil_offset(kind)]!=0; }
unsigned char color(Kind kind,const void* p) noexcept { return static_cast<const unsigned char*>(p)[nil_offset(kind)-1]; }
void color(Kind kind,void* p,unsigned char c) noexcept { static_cast<unsigned char*>(p)[nil_offset(kind)-1]=c; }
void subtree(Kind,void*,NativeStringStorage&);
void clear_tree(Kind kind,void* tree,NativeStringStorage& strings) {
    subtree(kind,link(link(tree,4),4),strings);
    put(link(tree,4),4,link(tree,4));write(tree,8,0);
    put(link(tree,4),0,link(tree,4));put(link(tree,4),8,link(tree,4));
}
void destroy_tree(Kind kind,void* tree,NativeStringStorage& strings) {
    clear_tree(kind,tree,strings);singleton_lifetime_free(link(tree,4));write(tree,4,0);write(tree,8,0);
}
enum class Vector { raw, strings, string_vectors };
void destroy_vector(Vector kind,void* header,NativeStringStorage& strings) {
    void* const first=link(header,4);
    if(first) {
        if(kind==Vector::strings) destroy_global_config_name_range_00432050(first,link(header,8),strings);
        else if(kind==Vector::string_vectors) {
            const Word end=read(header,8);
            for(Word p=address(first);p!=end;p+=16) destroy_vector(Vector::strings,pointer(p),strings);
        }
        singleton_lifetime_free(link(header,4));
    }
    write(header,4,0);write(header,8,0);write(header,12,0);
}
void device_record(void* self,NativeStringStorage& strings) {
    // 0055B690: reverse completed members, retaining each opaque header word.
    destroy_tree(Kind::int_only,at(self,0x78),strings);
    destroy_tree(Kind::bits,at(self,0x6c),strings);
    destroy_vector(Vector::string_vectors,at(self,0x5c),strings);
    destroy_tree(Kind::int_set,at(self,0x50),strings);
    destroy_vector(Vector::strings,at(self,0x40),strings);
    destroy_vector(Vector::strings,at(self,0x30),strings);
    destroy_tree(Kind::scalar_string_key,at(self,0x24),strings);
    destroy_tree(Kind::sensitivity,at(self,0x18),strings);
    destroy_tree(Kind::code_vector,at(self,0x0c),strings);
    destroy_tree(Kind::code_vector,self,strings);
}
void payload(Kind kind,void* node,NativeStringStorage& strings) {
    switch(kind) {
    case Kind::device:device_record(at(node,0x14),strings);break;
    case Kind::controller:destroy_tree(Kind::int_string,at(node,0x14),strings);break;
    case Kind::code_vector:destroy_vector(Vector::raw,at(node,0x14),strings);break;
    case Kind::sensitivity:destroy_vector(Vector::raw,at(node,0x18),strings);break;
    case Kind::bits:write(node,0x14,0);destroy_vector(Vector::raw,at(node,0x18),strings);break;
    case Kind::int_set:destroy_tree(Kind::defaults,at(node,0x10),strings);return;
    case Kind::int_string:destroy_native_string_header_0041dd20(at(node,0x10),strings);return;
    case Kind::defaults:case Kind::int_only:return;
    default:break;
    }
    destroy_native_string_header_0041dd20(at(node,0x0c),strings);
}
void subtree(Kind kind,void* node,NativeStringStorage& strings) {
    while(!nil(kind,node)) {
        subtree(kind,link(node,8),strings);
        void* const left=link(node);
        payload(kind,node,strings);singleton_lifetime_free(node);node=left;
    }
}
void* extreme(Kind kind,void* node,Word direction) noexcept {
    while(!nil(kind,link(node,direction))) node=link(node,direction);
    return node;
}
void advance(Kind kind,NativeKeyboardTreeIterator& iterator) {
    void* node=iterator.node;
    if(nil(kind,node)) _invalid_parameter_noinfo();
    if(!nil(kind,link(node,8))) node=extreme(kind,link(node,8),0);
    else {
        void* parent=link(node,4);
        while(!nil(kind,parent)&&node==link(parent,8)) { node=parent;parent=link(parent,4); }
        node=parent;
    }
    iterator.node=node;
}
void rotate(Kind kind,void* tree,void* node,Word direction) noexcept {
    const Word opposite=8-direction;
    void* const pivot=link(node,opposite);
    put(node,opposite,link(pivot,direction));
    if(!nil(kind,link(pivot,direction))) put(link(pivot,direction),4,node);
    put(pivot,4,link(node,4));
    if(node==link(link(tree,4),4)) put(link(tree,4),4,pivot);
    else if(node==link(link(node,4),direction)) put(link(node,4),direction,pivot);
    else put(link(node,4),opposite,pivot);
    put(pivot,direction,node);put(node,4,pivot);
}
struct CompletedString {
    NativeLegacySboStringStorage& value;
    ~CompletedString() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};
[[noreturn]] void invalid_iterator() {
    NativeLegacySboStringStorage value;value.capacity_18=15;value.length_14=0;value.buffer_04.inline_bytes[0]=0;
    native_legacy_sbo_string_assign_counted_00408720(value,"invalid map/set<T> iterator",27);
    const CompletedString completed{value};throw NativeHardwareLayoutInvalidIterator{value};
}
void erase_one(Kind kind,void* tree,void* removed,NativeKeyboardTreeIterator successor,NativeStringStorage& strings) {
    if(nil(kind,removed)) invalid_iterator();
    advance(kind,successor);
    void* replacement;
    void* transplant=removed;
    if(nil(kind,link(removed))) replacement=link(removed,8);
    else if(nil(kind,link(removed,8))) replacement=link(removed);
    else { transplant=successor.node;replacement=link(transplant,8); }
    void* parent;
    if(transplant==removed) {
        parent=link(removed,4);
        if(!nil(kind,replacement)) put(replacement,4,parent);
        if(link(link(tree,4),4)==removed) put(link(tree,4),4,replacement);
        else if(link(parent)==removed) put(parent,0,replacement);
        else put(parent,8,replacement);
        if(link(link(tree,4))==removed) put(link(tree,4),0,nil(kind,replacement)?parent:extreme(kind,replacement,0));
        if(link(link(tree,4),8)==removed) put(link(tree,4),8,nil(kind,replacement)?parent:extreme(kind,replacement,8));
    } else {
        put(link(removed),4,transplant);put(transplant,0,link(removed));
        if(transplant==link(removed,8)) parent=transplant;
        else {
            parent=link(transplant,4);
            if(!nil(kind,replacement)) put(replacement,4,parent);
            put(parent,0,replacement);put(transplant,8,link(removed,8));put(link(removed,8),4,transplant);
        }
        if(link(link(tree,4),4)==removed) put(link(tree,4),4,transplant);
        else if(link(link(removed,4))==removed) put(link(removed,4),0,transplant);
        else put(link(removed,4),8,transplant);
        put(transplant,4,link(removed,4));
        const auto original=color(kind,removed);color(kind,removed,color(kind,transplant));color(kind,transplant,original);
    }
    if(color(kind,removed)==1) {
        while(replacement!=link(link(tree,4),4)&&color(kind,replacement)==1) {
            const Word direction=replacement==link(parent)?0u:8u, opposite=8-direction;
            void* sibling=link(parent,opposite);
            if(color(kind,sibling)==0) {
                color(kind,sibling,1);color(kind,parent,0);rotate(kind,tree,parent,direction);sibling=link(parent,opposite);
            }
            if(nil(kind,sibling)) { replacement=parent;parent=link(parent,4);continue; }
            if(color(kind,link(sibling,direction))==1&&color(kind,link(sibling,opposite))==1) {
                color(kind,sibling,0);replacement=parent;parent=link(parent,4);continue;
            }
            if(color(kind,link(sibling,opposite))==1) {
                color(kind,link(sibling,direction),1);color(kind,sibling,0);rotate(kind,tree,sibling,opposite);sibling=link(parent,opposite);
            }
            color(kind,sibling,color(kind,parent));color(kind,parent,1);color(kind,link(sibling,opposite),1);
            rotate(kind,tree,parent,direction);break;
        }
        color(kind,replacement,1);
    }
    payload(kind,removed,strings);singleton_lifetime_free(removed);
    const Word count=read(tree,8);if(count) write(tree,8,count-1);
}
void* erase_range(Kind kind,void* tree,void* output,NativeKeyboardTreeIterator first,NativeKeyboardTreeIterator last,NativeStringStorage& strings) {
    void* const begin=link(link(tree,4));
    if(!first.owner||first.owner!=tree) _invalid_parameter_noinfo();
    if(first.node==begin) {
        void* const end=link(tree,4);
        if(!last.owner||last.owner!=tree) _invalid_parameter_noinfo();
        if(last.node==end) {
            clear_tree(kind,tree,strings);void* const result=link(link(tree,4));
            put(output,0,tree);put(output,4,result);return output;
        }
    }
    for(;;) {
        if(!first.owner||first.owner!=last.owner) _invalid_parameter_noinfo();
        if(first.node==last.node) break;
        const auto removed=first;advance(kind,first);erase_one(kind,tree,removed.node,removed,strings);
    }
    put(output,0,first.owner);put(output,4,first.node);return output;
}
} // namespace
void destroy_native_input_device_subtree_006a7540(void* /*tree*/,void* node,NativeStringStorage& strings) { subtree(Kind::device,node,strings); }
void destroy_native_input_device_record_0055b690(void* self,NativeStringStorage& strings) { device_record(self,strings); }
void* erase_native_input_device_tree_range_006a7aa0(void* t,void* o,NativeKeyboardTreeIterator a,NativeKeyboardTreeIterator b,NativeStringStorage& s) { return erase_range(Kind::device,t,o,a,b,s); }
void* erase_native_input_controller_tree_range_006a6a20(void* t,void* o,NativeKeyboardTreeIterator a,NativeKeyboardTreeIterator b,NativeStringStorage& s) { return erase_range(Kind::controller,t,o,a,b,s); }
void* erase_native_input_default_tree_range_0069fe70(void* t,void* o,NativeKeyboardTreeIterator a,NativeKeyboardTreeIterator b,NativeStringStorage& s) { return erase_range(Kind::defaults,t,o,a,b,s); }
void* erase_native_input_preset_tree_range_006a1aa0(void* t,void* o,NativeKeyboardTreeIterator a,NativeKeyboardTreeIterator b,NativeStringStorage& s) { return erase_range(Kind::preset,t,o,a,b,s); }
void* erase_native_input_scale_tree_range_0055b230(void* t,void* o,NativeKeyboardTreeIterator a,NativeKeyboardTreeIterator b,NativeStringStorage& s) { return erase_range(Kind::int_set,t,o,a,b,s); }
} // namespace bsp
