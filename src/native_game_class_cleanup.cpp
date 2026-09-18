#include "bsp/native_game_class_cleanup.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_input_settings_vector_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <Windows.h>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
static_assert(sizeof(void*)==4);
U address(const void* p) noexcept {return reinterpret_cast<U>(p);}
void* pointer(U p) noexcept {return reinterpret_cast<void*>(p);}
void* at(const void* p,U n=0) noexcept {return pointer(address(p)+n);}
volatile U& word(const void* p,U n=0) noexcept {return *static_cast<volatile U*>(at(p,n));}
void* ptr(const void* p,U n=0) noexcept {return pointer(word(p,n));}
void ptr(void* p,U n,void* value) noexcept {word(p,n)=address(value);}
bool nil(void* p) noexcept {return *static_cast<volatile unsigned char*>(at(p,0x19))!=0;}
void invalid(U site,NativeGameClassCleanupCalls& c,NativeGameClassCleanupProgress& o){o.native_site=site;c.class_invalid_parameter_00bf6713();}
void reset_tree(void* tree) noexcept {
    void* head=ptr(tree,4);ptr(head,4,head);head=ptr(tree,4);word(tree,8)=0;ptr(head,0,head);head=ptr(tree,4);ptr(head,8,head);
}
void* erase_range(void* h,void* out,void* fo,void* first,void* lo,void* last,
    NativeGameClassCleanupCalls& c,NativeGameClassCleanupProgress& o,U check_site,U move_site){
    if(!fo||fo!=lo)invalid(check_site,c,o);
    if(first!=last){
        const U difference=word(h,8)-address(last);const U bytes=difference&~3u;
        const U new_end=address(first)+bytes;
        if(difference>=4&&difference<0x80000000u){o.native_site=move_site;c.class_memmove_00bf67a7(first,bytes,last);}
        word(h,8)=new_end;
    }
    ptr(out,4,first);ptr(out,0,fo);return out;
}
void clear_vector(void* h,NativeGameClassCleanupCalls& c,NativeGameClassCleanupProgress& o,bool floating){
    const U begin=word(h,4);if(!begin)return;
    const U last=word(h,8);if(((last-begin)>>2)==0)return;
    const U delta=floating?0:0xc0;
    if(last<begin)invalid(0x4a8f81+delta,c,o);
    const U first=word(h,4);
    if(word(h,8)<first)invalid(0x4a8f8e+delta,c,o);
    if(word(h,8)<first||first<word(h,4))invalid(0x4a8fa4+delta,c,o);
    o.native_site=0x4a8fb4+delta;
    if(floating)erase_native_game_class_float_range_00488bf0(h,o.output,h,pointer(first),h,pointer(last),c,o);
    else erase_native_game_class_word_range_004a7a40(h,o.output,h,pointer(first),h,pointer(last),c,o);
}
U positive_zero_bits() noexcept {
    U value;
    __asm { fldz
        fstp dword ptr value
    }
    return value;
}
void normal(NativeGameClassCleanupContext& x,NativeGameClassCleanupOperation& o){
    auto& c=x.calls;o.native_site=0x4a9ac7;void* registry=c.class_registry_004a9890(x,o);o.registry=registry;
    void* owner=at(registry,4);void* node=ptr(ptr(registry,8));
    o.iterator[0]=address(owner);o.iterator[1]=address(node);
    for(;;){
        o.native_site=0x4a9ae0;void* current=c.class_registry_004a9890(x,o);void* end=ptr(current,8);
        if(!owner||owner!=at(current,4))invalid(0x4a9af3,c,o);
        if(node==end)break;
        if(!owner)invalid(0x4a9b00,c,o);
        if(node==ptr(owner,4))invalid(0x4a9b0a,c,o);
        o.cursor=node;
        if(void* payload=ptr(node,0x14)){o.native_site=0x4a9b1c;c.virtual_scalar(payload,0,1);word(node,0x14)=0;}
        if(node==ptr(owner,4))invalid(0x4a9b26,c,o);
        word(node,0x14)=0;o.native_site=0x4a9b32;c.class_increment_004a73a0(o.iterator,o);
        node=pointer(o.iterator[1]);owner=pointer(o.iterator[0]);
    }
    o.native_site=0x4a9b41;registry=c.class_registry_004a9890(x,o);o.registry=registry;
    void* tree=at(registry,4);o.tree=tree;o.cursor=ptr(ptr(registry,8),4);o.native_site=0x4a9b54;
    c.class_subtree_004a8ac0(tree,o.cursor,x,o);reset_tree(tree);
    void* vectors=x.actual_vectors_00e1875c;
    o.native_site=0x4a9b74;c.class_clear_words_004a8fd0(vectors,o);
    o.native_site=0x4a9b85;c.class_clear_float_004a8f10(at(vectors,0x10),positive_zero_bits(),o);
    o.native_site=0x4a9b96;c.class_clear_float_004a8f10(at(vectors,0x40),positive_zero_bits(),o);
    o.native_site=0x4a9ba2;c.class_clear_dwords_00492210(at(vectors,0x30));
    o.native_site=0x4a9bae;c.class_clear_dwords_00492210(at(vectors,0x50));
    o.native_site=0x4a9bbf;c.class_clear_float_004a8f10(at(vectors,0x20),positive_zero_bits(),o);
}
}
void* NativeGameClassCleanupCalls::class_allocate_00bf681b(U n){return singleton_lifetime_allocate({SingletonAllocationKind::object,n,n});}
void NativeGameClassCleanupCalls::class_free_00bf65ac(void* p){singleton_lifetime_free(p);}
void NativeGameClassCleanupCalls::class_invalid_parameter_00bf6713(){_invalid_parameter_noinfo();}
void NativeGameClassCleanupCalls::class_memmove_00bf67a7(void* destination,U bytes,const void* source){(void)memmove_s(destination,bytes,source,bytes);}
void* NativeGameClassCleanupCalls::class_manager_00415350(NativeStringRawPoolContext& x){return get_native_singleton_manager_00415350(x.actual_manager_publication_01090aa0);}
void NativeGameClassCleanupCalls::class_register_00bd0c30(void* m,void* p){register_native_singleton_object_00bd0c30(m,nullptr,p);}
void NativeGameClassCleanupCalls::class_enter_section(void* p){EnterCriticalSection(static_cast<CRITICAL_SECTION*>(p));}
void NativeGameClassCleanupCalls::class_leave_section(void* p){LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(p));}
NativeStringPoolStorage* NativeGameClassCleanupCalls::class_pool_00419cc0(NativeStringRawPoolContext& x){return native_string_pool_get_or_create_00419cc0(x.actual_published_01090aa8,x.actual_manager_publication_01090aa0);}
void NativeGameClassCleanupCalls::class_return_00bd1510(NativeStringPoolStorage* pool,void* p,U n,NativeStringRawPoolContext& x){return_native_string_pool_00bd1510(pool,p,n,x.actual_small_returns_disabled_01090aa4);}
void* NativeGameClassCleanupCalls::class_registry_004a9890(NativeGameClassCleanupContext& x,NativeGameClassCleanupProgress& o){return get_native_game_class_registry_004a9890(x,o);}
void NativeGameClassCleanupCalls::class_increment_004a73a0(void* p,NativeGameClassCleanupProgress& o){increment_native_game_class_iterator_004a73a0(p,*this,o);}
void NativeGameClassCleanupCalls::class_subtree_004a8ac0(void* tree,void* node,NativeGameClassCleanupContext& x,NativeGameClassCleanupProgress& o){destroy_native_game_class_subtree_004a8ac0(tree,node,x,o);}
void NativeGameClassCleanupCalls::class_clear_words_004a8fd0(void* p,NativeGameClassCleanupProgress& o){clear_native_game_class_word_vector_004a8fd0(p,*this,o);}
void NativeGameClassCleanupCalls::class_clear_float_004a8f10(void* p,U,NativeGameClassCleanupProgress& o){clear_native_game_class_float_vector_004a8f10(p,*this,o);}
void NativeGameClassCleanupCalls::class_clear_dwords_00492210(void* p){resize_native_input_settings_words_00492210(p,0,0);}
void* allocate_native_game_class_tree_node_004a7800(NativeGameClassCleanupCalls& c,NativeGameClassCleanupProgress& o){
    o.native_site=0x4a7802;void* p=c.class_allocate_00bf681b(0x1c);
    if(p)word(p)=0;if(at(p,4))word(p,4)=0;if(at(p,8))word(p,8)=0;
    *static_cast<volatile unsigned char*>(at(p,0x18))=1;*static_cast<volatile unsigned char*>(at(p,0x19))=0;return p;
}
void increment_native_game_class_iterator_004a73a0(void* iterator,NativeGameClassCleanupCalls& c,NativeGameClassCleanupProgress& o){
    if(!word(iterator))invalid(0x4a73a8,c,o);
    void* node=ptr(iterator,4);
    if(nil(node)){invalid(0x4a73b7,c,o);return;}
    void* right=ptr(node,8);
    if(!nil(right)){void* left=ptr(right);while(!nil(left)){right=left;left=ptr(right);}ptr(iterator,4,right);return;}
    void* parent=ptr(node,4);
    while(!nil(parent)&&ptr(iterator,4)==ptr(parent,8)){ptr(iterator,4,parent);parent=ptr(parent,4);}
    ptr(iterator,4,parent);
}
void destroy_native_game_class_subtree_004a8ac0(void* tree,void* node,NativeGameClassCleanupContext& x,NativeGameClassCleanupProgress& o){
    auto& c=x.calls;o.tree=tree;
    while(!nil(node)){
        o.cursor=node;o.native_site=0x4a8ad7;c.class_subtree_004a8ac0(tree,ptr(node,8),x,o);
        void* data=ptr(node,0x10);void* next=ptr(node);
        if(data){const U bytes=word(node,0xc)+1;o.cursor=node;o.native_site=0x4a8aef;auto* pool=c.class_pool_00419cc0(x.strings);
            o.native_site=0x4a8af6;c.class_return_00bd1510(pool,data,bytes,x.strings);}
        o.cursor=node;o.native_site=0x4a8afc;c.class_free_00bf65ac(node);node=next;
    }
}
void* clear_native_game_class_full_range_004a9240(void* tree,void* out,void* fo,void* first,void* lo,void* last,NativeGameClassCleanupContext& x,NativeGameClassCleanupProgress& o){
    void* head=ptr(tree,4);
    if(fo!=tree||lo!=tree||first!=ptr(head)||last!=head)throw std::invalid_argument("native class registry adapter requires its current full range");
    o.tree=tree;o.native_site=0x4a928a;x.calls.class_subtree_004a8ac0(tree,ptr(head,4),x,o);reset_tree(tree);
    ptr(out,0,tree);ptr(out,4,ptr(ptr(tree,4)));return out;
}
void destroy_native_game_class_registry_base_004a7140(void* p,NativeGameClassCleanupContext& x) noexcept{x.registry_00e187bc=nullptr;word(p)=0x00ce3818;}
void* construct_native_game_class_registry_004a9790(void* p,NativeGameClassCleanupContext& x,NativeGameClassCleanupProgress& o){
    o.registry=p;word(p)=0x00ce6c68;
    try{o.native_site=0x4a97c1;void* head=allocate_native_game_class_tree_node_004a7800(x.calls,o);ptr(p,8,head);
        *static_cast<volatile unsigned char*>(at(head,0x19))=1;
        head=ptr(p,8);ptr(head,4,head);head=ptr(p,8);ptr(head,0,head);head=ptr(p,8);ptr(head,8,head);word(p,0xc)=0;return p;
    }catch(...){destroy_native_game_class_registry_base_004a7140(p,x);throw;}
}
void* get_native_game_class_registry_004a9890(NativeGameClassCleanupContext& x,NativeGameClassCleanupProgress& o){
    void* initial=x.registry_00e187bc;if(initial)return initial;
    auto& c=x.calls;o.native_site=0x4a98b6;void* first=c.class_manager_00415350(x.strings);void* section=ptr(first,0x10);o.captured_section=section;
    U guard[2]{0x00ce37fc,address(section)};
    if(section){o.native_site=0x4a98cf;c.class_enter_section(section);word(section,0x18)=word(section,0x18)+1;}
    try{
        if(!x.registry_00e187bc){o.native_site=0x4a98ec;void* allocation=c.class_allocate_00bf681b(0x10);void* result;
            try{result=allocation?construct_native_game_class_registry_004a9790(allocation,x,o):nullptr;}
            catch(...){c.class_free_00bf65ac(allocation);throw;}
            x.registry_00e187bc=result;o.native_site=0x4a9916;void* second=c.class_manager_00415350(x.strings);
            void* current=x.registry_00e187bc;o.native_site=0x4a9924;c.class_register_00bd0c30(second,current);
        }
        if(section){word(section,0x18)=word(section,0x18)-1;o.native_site=0x4a9932;c.class_leave_section(section);}
    }catch(...){destroy_native_singleton_guard_00411ee0(guard);throw;}
    return x.registry_00e187bc;
}
void destroy_native_game_class_registry_004a9800(void* p,NativeGameClassCleanupContext& x,NativeGameClassCleanupProgress& o){
    o.registry=p;void* tree=at(p,4);void* head=ptr(p,8);
    try{o.native_site=0x4a983a;clear_native_game_class_full_range_004a9240(tree,o.output,tree,ptr(head),tree,head,x,o);
        o.native_site=0x4a9843;x.calls.class_free_00bf65ac(ptr(tree,4));word(tree,4)=0;word(tree,8)=0;
    }catch(...){destroy_native_game_class_registry_base_004a7140(p,x);throw;}
    destroy_native_game_class_registry_base_004a7140(p,x);
}
void* delete_native_game_class_registry_004a9870(void* p,U flags,NativeGameClassCleanupContext& x,NativeGameClassCleanupProgress& o){
    o.native_site=0x4a9873;destroy_native_game_class_registry_004a9800(p,x,o);if(flags&1){o.native_site=0x4a9880;x.calls.class_free_00bf65ac(p);}return p;
}
void delete_native_game_class_registered_owner(void* p,U flags,NativeGameClassCleanupContext& x){NativeGameClassCleanupProgress progress;delete_native_game_class_registry_004a9870(p,flags,x,progress);}
void* erase_native_game_class_float_range_00488bf0(void* h,void* out,void* fo,void* first,void* lo,void* last,NativeGameClassCleanupCalls& c,NativeGameClassCleanupProgress& o){return erase_range(h,out,fo,first,lo,last,c,o,0x488c03,0x488c2f);}
void* erase_native_game_class_word_range_004a7a40(void* h,void* out,void* fo,void* first,void* lo,void* last,NativeGameClassCleanupCalls& c,NativeGameClassCleanupProgress& o){return erase_range(h,out,fo,first,lo,last,c,o,0x4a7a53,0x4a7a7f);}
void clear_native_game_class_float_vector_004a8f10(void* p,NativeGameClassCleanupCalls& c,NativeGameClassCleanupProgress& o){clear_vector(p,c,o,true);}
void clear_native_game_class_word_vector_004a8fd0(void* p,NativeGameClassCleanupCalls& c,NativeGameClassCleanupProgress& o){clear_vector(p,c,o,false);}
NativeGameClassCleanupOperation::~NativeGameClassCleanupOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void NativeGameClassCleanupOperation::acknowledge_diagnostic_cleanup() noexcept{if(phase==Phase::failed)phase=Phase::diagnostic_retired;}
void clear_native_game_class_globals_004a9ac0(NativeGameClassCleanupContext& x,NativeGameClassCleanupOperation& o){
    if(o.phase!=NativeGameClassCleanupOperation::Phase::fresh)throw std::logic_error("native class globals cleanup cannot be replayed");
    o.context=&x;o.phase=NativeGameClassCleanupOperation::Phase::running;
    try{normal(x,o);o.phase=NativeGameClassCleanupOperation::Phase::complete;}catch(...){o.phase=NativeGameClassCleanupOperation::Phase::failed;throw;}
}
} // namespace bsp
