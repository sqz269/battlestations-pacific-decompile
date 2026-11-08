#include "bsp/native_game_resource_lists.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
static_assert(sizeof(void*)==4);
U bits(const void* p) noexcept {return static_cast<U>(reinterpret_cast<std::uintptr_t>(p));}
void* ptr(U value) noexcept {return reinterpret_cast<void*>(value);}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(ptr(bits(p)+offset));}
void put(void* p,U offset,U value) noexcept {*static_cast<volatile U*>(ptr(bits(p)+offset))=value;}
U sar2(U value) noexcept {std::int32_t s;std::memcpy(&s,&value,4);return static_cast<U>(s>>2);}
std::int32_t signed_bits(U value) noexcept {std::int32_t s;std::memcpy(&s,&value,4);return s;}
void invalid(const SingletonLifetimeCallbacks& c) {c.invalid_parameter(c.context);}
U size_from(void* list,U captured_begin) noexcept {return captured_begin?sar2(word(list,8)-captured_begin):0;}

void* allocate_impl(U count) {
    // Native unsigned division checks whether count*4 fits before BF681B.
    if(count>0x3fffffffu) throw std::bad_alloc();
    const U bytes=count*4u;
    return singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots,bytes,bytes});
}
void fill_range_impl(void* first,void* last,const void* value) {
    for(U p=bits(first);p!=bits(last);p+=4u) put(ptr(p),0,word(value));
}
void* copy_backward_impl(const void* first,const void* last,void* destination_end) {
    const U count=sar2(bits(last)-bits(first));
    const U bytes=count*4u;
    auto* const destination=ptr(bits(destination_end)-bytes);
    if(signed_bits(count)>0) (void)::memmove_s(destination,bytes,first,bytes);
    return destination;
}
void* copy_range_impl(const void* first,const void* last,void* destination) {
    const U count=sar2(bits(last)-bits(first));
    const U bytes=count*4u;
    auto* const result=ptr(bits(destination)+bytes);
    if(count!=0) (void)::memmove_s(destination,bytes,first,bytes);
    return result;
}
void* fill_copies_impl(void* destination,U count,const void* value) {
    U p=bits(destination),left=count;
    while(left!=0) {put(ptr(p),0,word(value));--left;p+=4u;}
    return ptr(bits(destination)+count*4u);
}
template<class Family>
void insert_copies_impl(void* list,void* position,U count,const void* value_address) {
    const U value=word(value_address); // Native overwrites its private argument spill.
    const U begin=word(list,4);
    U capacity=begin?sar2(word(list,0x0c)-begin):0;
    if(count==0) return;
    if(0x3fffffffu-size_from(list,begin)<count) throw std::length_error("vector<T> too long");
    if(capacity<size_from(list,begin)+count) {
        const U half=capacity>>1;
        capacity=0x3fffffffu-half<capacity?0:capacity+half;
        if(capacity<size_from(list,begin)+count) capacity=size_from(list,begin)+count;
        void* const fresh=Family::allocate(capacity);
        auto* next=Family::copy_range(ptr(word(list,4)),position,fresh);
        next=Family::fill_copies(next,count,&value);
        (void)Family::copy_range(position,ptr(word(list,8)),next);
        const U old_begin=word(list,4);
        const U new_count=count+size_from(list,old_begin);
        if(old_begin) singleton_lifetime_free(ptr(old_begin));
        // Original publishes begin, capacity end, then end, all from captures.
        put(list,4,bits(fresh));put(list,0x0c,bits(fresh)+capacity*4u);put(list,8,bits(fresh)+new_count*4u);
        return;
    }
    const U old_end=word(list,8),pos=bits(position),bytes=count*4u;
    if(sar2(old_end-pos)<count) {
        (void)Family::copy_range(position,ptr(old_end),ptr(pos+bytes));
        const U current_end=word(list,8);
        (void)Family::fill_copies(ptr(current_end),count-sar2(current_end-pos),&value);
        put(list,8,word(list,8)+bytes);
        const U fill_end=word(list,8)-bytes;
        Family::fill_range(position,ptr(fill_end),&value);
    } else {
        const U cut=old_end-bytes;
        auto* const new_end=Family::copy_range(ptr(cut),ptr(old_end),ptr(old_end));
        put(list,8,bits(new_end));
        (void)Family::copy_backward(position,ptr(cut),ptr(old_end));
        Family::fill_range(position,ptr(pos+bytes),&value);
    }
}
template<class Family>
void* insert_one_impl(void* list,void* output,void* owner,void* position,const void* value,
    const SingletonLifetimeCallbacks& callbacks) {
    const U begin=word(list,4);
    U offset=0;
    if(begin) {
        const U end=word(list,8);
        if(sar2(end-begin)!=0) {
            if(begin>end) invalid(callbacks);
            if(!owner || owner!=list) invalid(callbacks);
            offset=sar2(bits(position)-begin);
        }
    }
    Family::insert_copies(list,owner,position,1,value);
    const U new_begin=word(list,4);
    if(new_begin>word(list,8)) invalid(callbacks);
    const U result=new_begin+offset*4u;
    if(result>word(list,8) || result<word(list,4)) invalid(callbacks);
    put(output,4,result);put(output,0,bits(list));
    return output;
}
template<class Family>
void append_impl(void* list,const void* value,const SingletonLifetimeCallbacks& callbacks) {
    const U begin=word(list,4);
    const U size=begin?sar2(word(list,8)-begin):0;
    if(begin && size<sar2(word(list,0x0c)-begin)) {
        const U end=word(list,8);
        put(ptr(end),0,word(value));put(list,8,end+4u);
        return;
    }
    const U end=word(list,8);
    if(begin>end) invalid(callbacks);
    U result[2];
    (void)Family::insert_one(list,result,list,ptr(end),value,callbacks);
}
} // namespace

// The three native families are instruction-identical after substituting the
// audited family call targets. Their distinct entry names share this schedule.
// bsp.py show 0071B8D0/0071B230/0071A5A0 and callees 0071A760/0071A920:
// each append calls its own InsertOne, which calls its own InsertCopies;
// growth and range shifts call that family's existing allocate/copy/fill bodies.
// Keep those boundaries rather than bypassing them through the shared kernels.
// This composition alone does not activate the currently unbound game-resource
// classification owner; it supplies neither type-token publications nor items.
#define BSP_DEFINE_GAME_LIST_FAMILY(tag,alloc,fillrange,backward,copy,fill,insert,one,append) \
struct tag##_family { \
    static constexpr auto allocate=allocate_native_game_##tag##_pointers_##alloc; \
    static constexpr auto fill_range=fill_native_game_##tag##_range_##fillrange; \
    static constexpr auto copy_backward=copy_native_game_##tag##_backward_##backward; \
    static constexpr auto copy_range=copy_native_game_##tag##_range_##copy; \
    static constexpr auto fill_copies=fill_native_game_##tag##_copies_##fill; \
    static constexpr auto insert_copies=insert_native_game_##tag##_copies_##insert; \
    static constexpr auto insert_one=insert_native_game_##tag##_one_##one; \
}; \
void* allocate_native_game_##tag##_pointers_##alloc(U n) {return allocate_impl(n);} \
void fill_native_game_##tag##_range_##fillrange(void* a,void* b,const void* v) {fill_range_impl(a,b,v);} \
void* copy_native_game_##tag##_backward_##backward(const void* a,const void* b,void* d) {return copy_backward_impl(a,b,d);} \
void* copy_native_game_##tag##_range_##copy(const void* a,const void* b,void* d) {return copy_range_impl(a,b,d);} \
void* fill_native_game_##tag##_copies_##fill(void* d,U n,const void* v) {return fill_copies_impl(d,n,v);} \
void insert_native_game_##tag##_copies_##insert(void* l,void*,void* p,U n,const void* v) {insert_copies_impl<tag##_family>(l,p,n,v);} \
void* insert_native_game_##tag##_one_##one(void* l,void* o,void* owner,void* p,const void* v,const SingletonLifetimeCallbacks& c) {return insert_one_impl<tag##_family>(l,o,owner,p,v,c);} \
void append_native_game_##tag##_pointer_##append(void* l,const void* v,const SingletonLifetimeCallbacks& c) {append_impl<tag##_family>(l,v,c);}
BSP_DEFINE_GAME_LIST_FAMILY(type44,00714120,00716a50,00716a70,00718350,007188c0,0071a5a0,0071b230,0071b8d0)
BSP_DEFINE_GAME_LIST_FAMILY(type54,00714180,00716aa0,00716ac0,00718380,007188f0,0071a760,0071b2c0,0071b940)
BSP_DEFINE_GAME_LIST_FAMILY(type64,007141e0,00716af0,00716b10,007183b0,00718920,0071a920,0071b350,0071b9b0)
#undef BSP_DEFINE_GAME_LIST_FAMILY
} // namespace bsp
