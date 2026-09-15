#pragma once

#include "bsp/native_render_resource_record.hpp"

namespace bsp {

// Full 004C3020: no register inputs, EAX new raw 10h sentinel, RET.
// Allocate in the existing lifetime CRT domain; initialize next/previous to
// self, preserving the native separate address guards. String fields remain
// uninitialized. Exhausted allocation throws through that existing domain.
NativeRenderResourceAliasNode* allocate_native_render_alias_sentinel_004c3020();

// Full 004D0A10 through its hidden returning-free tail. ECX actual owner, RET.
// Clear aliases, reload and free the current sentinel, then null owner+4.
// Preserve owner+0 and callback changes to other fields. No null-owner guard.
void destroy_native_render_alias_list_004d0a10(void* actual_owner,
    SizedStoragePool& actual_string_pool);
// Actual owning-pool overload: every string operation uses the current 419CC0
// publication/gate/lifetime binding. Same native algorithm and exception limits.
void destroy_native_render_alias_list_004d0a10(void* actual_owner,
    ActualNativeStringPoolStorage& actual_string_pool);

// Full 004D48A0 plus catch004D490C. ECX destination, stack source, EAX
// destination, RET4. Publish new sentinel/count before reading source links;
// identity therefore copies the newly empty list and abandons the old graph.
// On range-copy failure destroy the CURRENT actual list, then rethrow; keep the
// existing range helper's own rollback behavior and exception domain.
void* copy_construct_native_render_alias_list_004d48a0(void* actual_destination_owner,
    const void* actual_source_owner, SizedStoragePool& actual_string_pool,
    const SingletonLifetimeCallbacks&);
// Actual owning-pool overload: every string operation uses the current 419CC0
// publication/gate/lifetime binding. Same native algorithm and exception limits.
void* copy_construct_native_render_alias_list_004d48a0(void* actual_destination_owner,
    const void* actual_source_owner, ActualNativeStringPoolStorage& actual_string_pool,
    const SingletonLifetimeCallbacks&);

// Full 00B2FC60: ECX destination, stack source, EAX destination, RET4.
// Zero name fields before the identity branch, copy its current fields, then
// construct actual aliases. Only successful name construction arms its cleanup.
// Copy five payload DWORDs and the unretained resource pointer after aliases.
// Preserve unknown_08. No old-output release, resource AddRef, or rollback.
// Evidence and explicit C++/storage/EH boundaries:
// docs/NATIVE_RENDER_RESOURCE_RECORD_CONSTRUCTION.md.
NativeRenderResourceRecord& copy_construct_native_render_resource_record_00b2fc60(
    NativeRenderResourceRecord& actual_destination,
    const NativeRenderResourceRecord& actual_source, SizedStoragePool& actual_string_pool,
    const SingletonLifetimeCallbacks&);
NativeRenderResourceRecord& copy_construct_native_render_resource_record_00b2fc60(
    NativeRenderResourceRecord&, const NativeRenderResourceRecord&,
    ActualNativeStringPoolStorage&, const SingletonLifetimeCallbacks&);


// Raw publication overloads preserve getter failure propagation. The list-copy
// catch destroys the current list and may replace the original exception.
void destroy_native_render_alias_list_004d0a10(void*, NativeStringRawPoolContext&);
void* copy_construct_native_render_alias_list_004d48a0(void*, const void*, NativeStringRawPoolContext&);

} // namespace bsp
