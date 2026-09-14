#include "bsp/native_loading_queue_submission.hpp"
#include "bsp/native_loading_queue_work_items.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <new>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native loading queue submission requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U=std::uint32_t;
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U v) noexcept { return reinterpret_cast<void*>(v); }
void* at(const void* p,U n=0) noexcept { return ptr(bits(p)+n); }
U word(const void* p,U n=0) noexcept { return *static_cast<const volatile U*>(at(p,n)); }
void put(void* p,U n,U v) noexcept { *static_cast<volatile U*>(at(p,n))=v; }
std::int32_t signed_word(U v) noexcept { std::int32_t result;std::memcpy(&result,&v,sizeof(result));return result; }
void return_captured(void* data,U bytes,NativeStringRawPoolContext& raw) {
    auto* const pool=native_string_pool_get_or_create_00419cc0(raw.actual_published_01090aa8,raw.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool,data,bytes,raw.actual_small_returns_disabled_01090aa4);
}
struct NameUnwind {
    void* header;
    NativeStringRawPoolContext& raw;
    bool armed=true;
    ~NameUnwind() noexcept(false) { if(armed) destroy_native_string_header_0041dd20(header,raw); }
};
struct PlacementUnwind {
    void* vector;
    void* original;
    bool armed=true;
    ~PlacementUnwind() noexcept {
        if(armed) {
            const auto offset=word(vector,4)<<4;
            auto* const current=ptr(offset+word(vector));
            // C68FF0 reads current count/data before the RET-only401130 call.
            ::operator delete(original,current);
        }
    }
};
struct WorkItemUnwind {
    void* item;
    NativeStringRawPoolContext& raw;
    bool armed=true;
    ~WorkItemUnwind() noexcept(false) { if(armed) destroy_native_loading_work_item_004fdc00(item,raw); }
};
}

void destroy_native_loading_work_item_004fdc00(void* item,NativeStringRawPoolContext& raw) {
    auto* const data=ptr(word(item,4));
    if(data) { const auto count=word(item)+1u;return_captured(data,count,raw); }
}
void* construct_native_loading_work_item_004fe700(void* output,void* arguments,NativeStringRawPoolContext& raw) {
    auto* const data=ptr(word(arguments,4));
    const auto length=word(arguments);
    const bool identical=output==arguments;
    NameUnwind parameter{arguments,raw}; // Native state0 owns the by-value name.
    put(output,0,0);put(output,4,0);
    if(!identical) {
        resize_native_string_header_0041dd40(output,raw,length,true);
        if(length!=0) {
            const auto count=word(output);auto* const destination=ptr(word(output,4));
            if(count!=0) std::memmove(destination,data,count);
        }
    }
    const auto context=word(arguments,8);put(output,8,context);put(output,12,0);
    parameter.armed=false;
    if(data) return_captured(data,length+1u,raw);
    return output;
}
void* copy_construct_native_loading_work_item_00501720(void* output,const void* source,NativeStringRawPoolContext& raw) {
    const bool identical=output==source;put(output,0,0);put(output,4,0);
    if(!identical) {
        const auto length=word(source);resize_native_string_header_0041dd40(output,raw,length,true);
        if(word(source)!=0) {
            const auto count=word(output);auto* const input=ptr(word(source,4));auto* const destination=ptr(word(output,4));
            if(count!=0) std::memmove(destination,input,count);
        }
    }
    const auto context=word(source,8);put(output,8,context);
    const auto result=word(source,12);put(output,12,result);
    return output;
}
void append_native_loading_work_item_005048f0(void* vector,const void* source,NativeStringRawPoolContext& raw) {
    const auto capacity=word(vector,8);
    if(word(vector,4)==capacity) {
        auto next=capacity+capacity;if(signed_word(next)<=1) next=1;
        reserve_native_loading_work_items_005018a0(vector,raw,signed_word(next));
    }
    const auto offset=word(vector,4)<<4;auto* const destination=ptr(offset+word(vector));
    PlacementUnwind placement{vector,destination};
    if(destination) copy_construct_native_loading_work_item_00501720(destination,source,raw);
    put(vector,4,word(vector,4)+1u);placement.armed=false;
}
void submit_native_loading_work_item_00504d20(void* loader,const void* name,void* callback,NativeStringRawPoolContext& raw) {
    const auto initial_count=word(loader,0x14);const auto initial_array=word(loader,0x10);
    auto* const first_selection=ptr(word(ptr(initial_array+initial_count*4u-4u)));
    if(*static_cast<const volatile unsigned char*>(at(first_selection,4))!=0) return;
    U arguments[3];arguments[2]=bits(callback);
    const bool identical=arguments==name;arguments[0]=0;arguments[1]=0;
    if(!identical) {
        const auto length=word(name);resize_native_string_header_0041dd40(arguments,raw,length,true);
        if(word(name)!=0) {
            const auto count=word(arguments);auto* const source=ptr(word(name,4));auto* const destination=ptr(word(arguments,4));
            if(count!=0) std::memmove(destination,source,count);
        }
    }
    U temporary[4];
    auto* const completed=construct_native_loading_work_item_004fe700(temporary,arguments,raw);
    const auto current_array=word(loader,0x10);const auto current_count=word(loader,0x14);
    auto* const current_last=ptr(word(ptr(current_array+current_count*4u-4u)));
    WorkItemUnwind item{temporary,raw};
    append_native_loading_work_item_005048f0(at(current_last,8),completed,raw);
    auto* const data=ptr(word(temporary,4));item.armed=false;
    if(data) { const auto bytes=word(temporary)+1u;return_captured(data,bytes,raw); }
}
} // namespace bsp
