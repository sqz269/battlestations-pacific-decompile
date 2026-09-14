#include "bsp/native_resource_node_traversal.hpp"
#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource node traversal requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U v) noexcept { return reinterpret_cast<void*>(v); }
void* at(const void* p,U n=0) noexcept { return ptr(bits(p)+n); }
U word(const void* p,U n=0) noexcept { return *static_cast<const volatile U*>(at(p,n)); }
void put(void* p,U n,U v) noexcept { *static_cast<volatile U*>(at(p,n))=v; }
void copy_tag(void* output,const void* source,NativeStringRawPoolContext& strings,bool destination_first) {
    if(output==source) return;
    const auto length=word(source);resize_native_string_header_0041dd40(output,strings,length,true);
    if(word(source)!=0) {
        const auto count=word(output);void* destination;void* data;
        if(destination_first) { destination=ptr(word(output,4));data=ptr(word(source,4)); }
        else { data=ptr(word(source,4));destination=ptr(word(output,4)); }
        if(count!=0) std::memmove(destination,data,count);
    }
}
}
void* construct_native_resource_child_00bea250(void* node,void* parent,NativeResourceStreamReadContext& context) {
    put(node,0,0x00ceb130);put(node,4,1);put(node,0,0x00d68bb4);
    auto* const reader=ptr(word(parent,8));auto* const tag=at(node,0x10);
    put(node,8,bits(reader));put(node,12,bits(parent));put(tag,0,0);put(tag,4,0);
    auto* const first_parent=ptr(word(node,12));const auto depth=word(parent,0x18)+1u;
    auto* const first_reader=ptr(word(node,8));U temporary[2];int state=1;
    put(node,0x18,depth);
    try {
        auto* const result=read_native_resource_string_00bf0510(first_reader,temporary,
            static_cast<U*>(at(first_parent,0x20)),context);
        state=2;copy_tag(tag,result,context.strings,true);
        auto* const data=ptr(word(temporary,4));state=1;
        if(data) {
            const auto bytes=word(temporary)+1u;
            auto* const pool=native_string_pool_get_or_create_00419cc0(context.strings.actual_published_01090aa8,
                context.strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool,data,bytes,context.strings.actual_small_returns_disabled_01090aa4);
        }
        auto* const next_parent=ptr(word(node,12));auto* const next_reader=ptr(word(node,8));
        const auto payload=read_native_resource_dword_00bf0280(next_reader,static_cast<U*>(at(next_parent,0x20)),context);
        put(node,0x1c,payload);put(node,0x20,payload);
        auto* const current_reader=ptr(word(node,8));const auto index=word(current_reader,0x60);
        auto* const path=at(current_reader,0x10+index*8u);
        put(current_reader,0x60,index+1u);copy_tag(path,tag,context.strings,false);
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
void* create_native_resource_child_00bea680(void* parent_handle,void* output,NativeResourceStreamReadContext& context) {
    auto* const allocation=singleton_lifetime_allocate({SingletonAllocationKind::object,0x24,0x24});
    void* result=nullptr;
    try { if(allocation) result=construct_native_resource_child_00bea250(allocation,ptr(word(parent_handle)),context); }
    catch(...) { singleton_lifetime_free(allocation);throw; }
    put(output,0,bits(result));return output;
}
U read_native_resource_control_dword_00bf02a0(void* reader,U* budget,NativeResourceStreamReadContext& context) {
    U actual=bits(reader);auto* const stream=ptr(word(reader));const auto entry=word(ptr(word(stream)),0x38);
    U value;
    switch(entry) {
    case 0x00be42e0:value=read_native_stream_dword_slot34_00be42e0(stream,&actual,context.streams);break;
    case 0x00be4300:value=read_native_stream_dword_slot38_00be4300(stream,&actual,context.streams);break;
    default:throw std::runtime_error("Reached native resource control scalar target is not reconstructed");
    }
    put(budget,0,word(budget)-actual);return value;
}
U seek_native_resource_relative_00bf03e0(void* reader,U low_offset,void*,NativeResourceStreamReadContext& context) {
    auto* const stream=ptr(word(reader));auto* const table=ptr(word(stream));const auto entry=word(table,0x1c);
    return context.streams.source_seek(entry,stream,low_offset,0,1);
}
void read_native_resource_node_control_00be9a40(void* handle,NativeResourceStreamReadContext& context) {
    auto* const node=ptr(word(handle));auto* const reader=ptr(word(node,8));
    const auto control=read_native_resource_control_dword_00bf02a0(reader,static_cast<U*>(at(node,0x20)),context);
    put(reader,0x64,control);
}
void skip_native_resource_node_00be9c40(void* handle,NativeResourceStreamReadContext& context) {
    auto* const node=ptr(word(handle));auto* const parent=ptr(word(node,12));
    if(parent) { const auto declared=word(node,0x1c);put(parent,0x20,word(parent,0x20)-declared); }
    const auto remaining=word(node,0x20);
    if(remaining!=0) {
        auto* const reader=ptr(word(node,8));U ignored=0;
        seek_native_resource_relative_00bf03e0(reader,remaining,&ignored,context);put(node,0x20,0);
    }
    auto* const reader=ptr(word(node,8));put(reader,0x60,word(reader,0x60)-1u);put(node,8,0);
}
bool native_resource_node_has_remaining_00715bf0(const void* handle) noexcept {
    auto* const node=ptr(word(handle));return node && word(node,0x20)!=0;
}
}
