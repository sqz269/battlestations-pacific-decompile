#include "bsp/native_resource_root_owner.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource root owners require MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U v) noexcept { return reinterpret_cast<void*>(v); }
void* at(const void* p,U n=0) noexcept { return ptr(bits(p)+n); }
U word(const void* p,U n=0) noexcept { return *static_cast<const volatile U*>(at(p,n)); }
void put(void* p,U n,U v) noexcept { *static_cast<volatile U*>(at(p,n))=v; }
void copy_current(void* output,const void* source,NativeStringRawPoolContext& strings) {
    if(output==source) return;
    const auto length=word(source);resize_native_string_header_0041dd40(output,strings,length,true);
    if(word(source)!=0) {
        const auto count=word(output);auto* const data=ptr(word(source,4));auto* const destination=ptr(word(output,4));
        if(count!=0) std::memmove(destination,data,count);
    }
}
void give_back(void* data,U bytes,NativeStringRawPoolContext& context) {
    auto* const pool=native_string_pool_get_or_create_00419cc0(context.actual_published_01090aa8,context.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool,data,bytes,context.actual_small_returns_disabled_01090aa4);
}
}
void* construct_native_resource_root_00bea380(void* node,void* reader,NativeResourceStreamReadContext& context) {
    put(node,0,0x00ceb130);put(node,4,1);
    auto* const tag=at(node,0x10);
    put(node,0,0x00d68bb4);put(node,8,bits(reader));put(node,12,0);
    put(tag,0,0);put(tag,4,0);put(node,0x18,0);
    U budget=1000,temporary[2];int state=1;
    try {
        auto* const result=read_native_resource_string_00bf0510(reader,temporary,&budget,context);
        state=2;copy_current(tag,result,context.strings);
        auto* const data=ptr(word(temporary,4));state=1;
        if(data) { const auto bytes=word(temporary)+1u;give_back(data,bytes,context.strings); }
        const auto payload=read_native_resource_dword_00bf0280(reader,&budget,context);
        put(node,0x1c,payload);put(node,0x20,payload);
        auto* const current_reader=ptr(word(node,8));const auto index=word(current_reader,0x60);
        auto* const path=at(current_reader,0x10+index*8u);
        put(current_reader,0x60,index+1u);copy_current(path,tag,context.strings);
        return node;
    } catch(...) {
        try {
            if(state==2) destroy_native_string_header_0041dd20(temporary,context.strings);
            destroy_native_string_header_0041dd20(tag,context.strings);
            destroy_native_ref_counted_base_00bd30f0(node);
        } catch(...) { std::terminate(); }
        throw;
    }
}
void* create_native_resource_root_00bea700(void* reader,void* output,NativeResourceStreamReadContext& context) {
    auto* const allocation=singleton_lifetime_allocate({SingletonAllocationKind::object,0x24,0x24});
    void* result=nullptr;
    try { if(allocation) result=construct_native_resource_root_00bea380(allocation,reader,context); }
    catch(...) { singleton_lifetime_free(allocation);throw; }
    put(output,0,bits(result));return output;
}
void destroy_native_resource_node_00be9df0(void* node,NativeStringRawPoolContext& strings) {
    put(node,0,0x00d68bb4);
    try {
        if(word(node,8)!=0) {
            auto* const parent=ptr(word(node,12));
            if(parent) { const auto declared=word(node,0x1c);put(parent,0x20,word(parent,0x20)-declared); }
            auto* const reader=ptr(word(node,8));put(reader,0x60,word(reader,0x60)-1u);put(node,8,0);
        }
        auto* const data=ptr(word(node,0x14));
        if(data) { const auto count=word(node,0x10)+1u;give_back(data,count,strings); }
    } catch(...) { destroy_native_ref_counted_base_00bd30f0(node);throw; }
    destroy_native_ref_counted_base_00bd30f0(node);
}
void* delete_native_resource_node_00be9fc0(void* node,U flags,NativeStringRawPoolContext& strings) {
    destroy_native_resource_node_00be9df0(node,strings);
    if((flags&1u)!=0) singleton_lifetime_free(node);
    return node;
}
NativeResourceRootDispatch::NativeResourceRootDispatch(NativeAdoptedSubstreamDispatch& s,NativeStringRawPoolContext& p):streams_(s),strings_(p) {}
std::uint8_t NativeResourceRootDispatch::source_is_open(std::uintptr_t e,void* s) { return streams_.source_is_open(e,s); }
U NativeResourceRootDispatch::source_seek(std::uintptr_t e,void* s,U lo,U hi,U origin) { return streams_.source_seek(e,s,lo,hi,origin); }
void NativeResourceRootDispatch::source_read(std::uintptr_t e,void* s,void* d,U n,U* a) { streams_.source_read(e,s,d,n,a); }
void NativeResourceRootDispatch::source_write(std::uintptr_t e,void* s,const void* d,U n,U* a) { streams_.source_write(e,s,d,n,a); }
void NativeResourceRootDispatch::source_zero_reference(std::uintptr_t e,void* owner,std::uintptr_t table) {
    if(e==0x00bd30e0 && table==0x00d68bb4) {
        if(!owner) return;
        const auto target=word(ptr(word(owner)),4);
        if(target!=0x00be9fc0) throw std::runtime_error("Reached native resource-node deleting target is not reconstructed");
        delete_native_resource_node_00be9fc0(owner,1,strings_);return;
    }
    streams_.source_zero_reference(e,owner,table);
}
}
