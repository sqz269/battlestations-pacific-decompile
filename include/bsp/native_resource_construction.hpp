#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// B88260: ECX raw44h-or-larger object; stack actual8h name-header pointer;
// EAX same object; RET4. This source interface adds the actual pool context.
// The caller supplies raw storage. Self-name alias is compared before zeroing
// the destination header. Name-copy failure only stamps the ref-counted base;
// it does not return a partially allocated name or initialize later fields.
void* construct_native_resource_base_00b88260(void* actual_object,
    const void* actual_name, NativeStringRawPoolContext&);

// 71B810: ECX raw74h object; stack actual name pointer; EAX object; RET4.
// Base construction then CFD8CC and three zeroed pointer triplets. Words44h,
// 54h and64h remain untouched. Its native EH state stays-1 throughout.
void* construct_native_game_resource_0071b810(void* actual_object,
    const void* actual_name, NativeStringRawPoolContext&);

// 71B870/B88340: original ECX factory is unused; stack actual name pointer;
// EAX allocation/null; RET4. Allocate74h/44h, construct and free the exact
// saved allocation if construction throws. No cache/global publication occurs.
// These return raw refcount1 resources; successful-resource destruction and
// actual load/cache composition remain incomplete. No implicit owning wrapper,
// callable numeric vtable or original FH3/SEH ABI is supplied here.
void* create_native_game_resource_0071b870(const void* actual_name,
    NativeStringRawPoolContext&);
void* create_native_default_resource_00b88340(const void* actual_name,
    NativeStringRawPoolContext&);
} // namespace bsp
