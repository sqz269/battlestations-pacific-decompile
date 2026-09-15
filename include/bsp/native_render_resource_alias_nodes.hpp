#pragma once
#include "bsp/native_render_resource_record.hpp"

namespace bsp {

// Complete 0044BCB0: original ECX=destination eight-byte string header,
// EDX=source header, RET0; no semantic return value. Actual headers are
// {uint32 length, char* data}; this byte-address interface also operates on
// AliasNode's existing +08/+0C members without overlaying another host object.
//
// A null destination is a native no-op. Otherwise clear destination length/data
// BEFORE acting on identity equality; self-construction therefore abandons an
// existing buffer. This is construction, not assignment or an ownership guard.
// Source bytes/fields are read again after resizing. The caller supplies the
// same actual 00419CC0 string-pool domain and valid readable/writable storage.
// Native construction's placement-delete unwind leaf is a verified no-op;
// no buffer release or header rollback is added when copying throws.
void construct_native_render_alias_string_0044bcb0(void* actual_destination_header,
    const void* actual_source_header, SizedStoragePool& actual_string_pool);
// Actual owning-pool overload: every string operation uses the current 419CC0
// publication/gate/lifetime binding. Same native algorithm and exception limits.
void construct_native_render_alias_string_0044bcb0(void* actual_destination_header,
    const void* actual_source_header, ActualNativeStringPoolStorage& actual_string_pool);

// Complete 004CE6F0 plus catch004CE75C. Original ECX/EDX are not inputs;
// stack {next, previous, source-string-header}, EAX=actual new10h node, RET0C.
// Allocate through the native CRT domain, establish the SAME AliasNode layout,
// store next/previous, and construct its actual embedded string. A copy failure
// frees the captured raw node and rethrows. Allocation failure precedes that
// catch. No list links/count, checked iterator, insertion, or later count-error
// cleanup belongs here; successful ownership transfers to the caller.
NativeRenderResourceAliasNode* allocate_native_render_alias_node_004ce6f0(
    NativeRenderResourceAliasNode* next, NativeRenderResourceAliasNode* previous,
    const void* actual_source_string_header, SizedStoragePool& actual_string_pool);
// Actual owning-pool overload: every string operation uses the current 419CC0
// publication/gate/lifetime binding. Same native algorithm and exception limits.
NativeRenderResourceAliasNode* allocate_native_render_alias_node_004ce6f0(
    NativeRenderResourceAliasNode* next, NativeRenderResourceAliasNode* previous,
    const void* actual_source_string_header, ActualNativeStringPoolStorage& actual_string_pool);


// Raw publication overloads. Getter failures propagate; no noexcept adapter or
// injected validation callback is interposed. Same actual header/node storage.
void construct_native_render_alias_string_0044bcb0(void*, const void*, NativeStringRawPoolContext&);
NativeRenderResourceAliasNode* allocate_native_render_alias_node_004ce6f0(
    NativeRenderResourceAliasNode*, NativeRenderResourceAliasNode*, const void*, NativeStringRawPoolContext&);

} // namespace bsp
