#include "bsp/native_session_message55_library.hpp"
#include <cstring>
#include <list>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace bsp {
namespace {
using U=std::uint32_t;using Row=NativeMessage55Receive;
void copy_float(U* destination,const U* source){__asm {mov eax,source
 mov ecx,destination
 fld dword ptr [eax]
 fstp dword ptr [ecx]}}
// Source-only allocator/element context, not an invented native process global.
// STL owns its iteration; scoped binding supplies the already-required pool.
thread_local NativeStringRawPoolContext* current_strings;
void invoke_with_pool(NativeStringRawPoolContext& strings,void (*operation)(void*),void* state){
    auto* previous=current_strings;current_strings=&strings;
    __try{operation(state);}__finally{current_strings=previous;}
}
void require_layout(){
    if(sizeof(void*)!=4||_ITERATOR_DEBUG_LEVEL!=0)throw std::logic_error("Native message55 STL binding requires Win32 iterator-debug-level0");
}
template<class T,U Limit> struct RawAllocator {
    using value_type=T;using size_type=U;using difference_type=std::int64_t;
    using is_always_equal=std::true_type;
    template<class Other> struct rebind {using other=RawAllocator<Other,Limit>;};
    RawAllocator() noexcept=default;
    template<class Other> RawAllocator(const RawAllocator<Other,Limit>&) noexcept{}
    T* allocate(U count){
        if(count>0xffffffffu/sizeof(T))throw std::bad_alloc();
        const U bytes=count*static_cast<U>(sizeof(T));
        auto* result=static_cast<T*>(singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes}));
        if(result==nullptr&&count!=0)throw std::bad_alloc();return result;
    }
    void deallocate(T* allocation,U) noexcept{singleton_lifetime_free(allocation);}
    U max_size() const noexcept{return Limit;}
};
template<class T,class V,U L> bool operator==(const RawAllocator<T,L>&,const RawAllocator<V,L>&) noexcept{return true;}
template<class T,class V,U L> bool operator!=(const RawAllocator<T,L>&,const RawAllocator<V,L>&) noexcept{return false;}
struct AdoptValue {};
struct ReceiveValue : Row {
    // The caller has already constructed the by-value argument. Adopt its
    // representation, including indeterminate inactive fields, for destruction
    // at function exit; normal message55 callers supply a zero string header.
    ReceiveValue(AdoptValue,const Row& row) noexcept{std::memcpy(static_cast<Row*>(this),&row,sizeof(Row));}
    ReceiveValue(const ReceiveValue& source){copy_construct_native_message55_receive_008e0950(this,&source,*current_strings);}
    ~ReceiveValue() noexcept(false){destroy_native_message55_receive_008ddfe0(this,*current_strings);}
    ReceiveValue& operator=(const ReceiveValue&)=delete;
};
using List=std::list<void*,RawAllocator<void*,0x3fffffffu>>;
using Vector=std::vector<ReceiveValue,RawAllocator<ReceiveValue,0x09249249u>>;
using ListNode=std::_List_node<void*,void*>;
static_assert(std::is_standard_layout_v<ReceiveValue> && sizeof(ReceiveValue)==sizeof(Row));
static_assert(sizeof(ListNode)==sizeof(NativeMessage55Node));
static_assert(offsetof(ListNode,_Next)==0 && offsetof(ListNode,_Prev)==4 && offsetof(ListNode,_Myval)==8);
static_assert(_ITERATOR_DEBUG_LEVEL!=0 || sizeof(List)==8);
static_assert(_ITERATOR_DEBUG_LEVEL!=0 || sizeof(Vector)==12);
List* list_at(NativeMessage55List* p){return reinterpret_cast<List*>(&p->sentinel_04);}
Vector* vector_at(NativeMessage55Vector* p){return reinterpret_cast<Vector*>(&p->begin_04);}
NativeMessage55Node* construct_list(NativeMessage55List* p){require_layout();new(static_cast<void*>(&p->sentinel_04)) List;return p->sentinel_04;}
void destroy_list(NativeMessage55List* p){require_layout();list_at(p)->~List();p->sentinel_04=nullptr;p->count_08=0;}
struct ResizeState {NativeMessage55Vector* vector;U count;const Row* value;};
void resize_vector(void* state){
    const auto& s=*static_cast<ResizeState*>(state);ReceiveValue argument(AdoptValue{},*s.value);
    if(s.vector->begin_04==nullptr)new(static_cast<void*>(&s.vector->begin_04)) Vector;
    vector_at(s.vector)->resize(s.count,argument);
}
struct DestroyState {Row* begin;Row* end;};
void destroy_range(void* state){const auto& s=*static_cast<DestroyState*>(state);std::destroy(reinterpret_cast<ReceiveValue*>(s.begin),reinterpret_cast<ReceiveValue*>(s.end));}
}
NativeMessage55Receive* copy_construct_native_message55_receive_008e0950(Row* destination,const Row* source,NativeStringRawPoolContext& strings){
    volatile auto& d=*destination;const volatile auto& s=*source;d.length_00=0;d.data_04=nullptr;
    if(destination!=source){
        resize_native_string_header_0041dd40(&destination->length_00,strings,s.length_00,true);
        if(s.length_00!=0){const U bytes=d.length_00;const char* input=s.data_04;char* output=d.data_04;std::memmove(output,input,bytes);}
    }
    d.has_handle_08=s.has_handle_08;d.handle_0a=s.handle_0a;d.resolved_0c=s.resolved_0c;
    copy_float(&destination->position_10[0],&source->position_10[0]);
    copy_float(&destination->position_10[1],&source->position_10[1]);
    copy_float(&destination->position_10[2],&source->position_10[2]);return destination;
}
void destroy_native_message55_receive_008ddfe0(Row* row,NativeStringRawPoolContext& strings){destroy_native_string_header_0041dd20(&row->length_00,strings);}
NativeMessage55StandardLibrary::NativeMessage55StandardLibrary(NativeStringRawPoolContext& strings):strings_(&strings){require_layout();}
NativeMessage55Node* NativeMessage55StandardLibrary::sentinel_008db560(NativeMessage55List* p){return construct_list(p);}
NativeMessage55Node* NativeMessage55StandardLibrary::sentinel_008db4f0(NativeMessage55List* p){return construct_list(p);}
void NativeMessage55StandardLibrary::resize_entries_008e0ec0(NativeMessage55List* p,U count,void* value){require_layout();list_at(p)->resize(count,value);}
void NativeMessage55StandardLibrary::resize_received_008e17c0(NativeMessage55Vector* p,U count,const Row& value){
    require_layout();ResizeState state{p,count,&value};invoke_with_pool(*strings_,&resize_vector,&state);
}
void NativeMessage55StandardLibrary::destroy_received_008de660(Row* begin,Row* end,NativeMessage55Vector*,NativeSessionMessage55*){
    require_layout();DestroyState state{begin,end};
    // The game caller owns the subsequent free and header clears. std::destroy
    // invokes only value destructors and leaves the supplied header unchanged.
    invoke_with_pool(*strings_,&destroy_range,&state);
}
void NativeMessage55StandardLibrary::destroy_entries_008db580(NativeMessage55List* p){destroy_list(p);}
void NativeMessage55StandardLibrary::unwind_entries_008dbab0(NativeMessage55List* p){destroy_list(p);}
} // namespace bsp
