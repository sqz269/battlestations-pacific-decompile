#pragma once

#include "bsp/native_render_alias_checked_ops.hpp"

namespace bsp {

// Full004D2660, ECX actual destination list, stack source8byte pooled-string
// header, RET4, no semantic return. Capture the current sentinel/previous,
// allocate an actual node, grow count, then publish the two links. Count-growth
// failure does not free the successful unlinked node: no local cleanup exists.
void append_native_render_alias_004d2660(void* actual_destination_owner,
    const void* actual_source_string_header, SizedStoragePool& actual_string_pool);

// Full logical004D26A0 through004D27B3, including parent catch004D2746.
// Native ECX destination; stack three iterator pairs plus one unread DWORD;
// RET1Ch, no semantic return. These iterator arguments are copied by value,
// as in the native frame. Their working words remain mutable during rollback.
// The unread native word has no invented semantic counterpart in this API.
//
// Insert copied source strings before the captured insertion node. Inlined
// checks retain the native owner/node captures and use the existing returning
// or throwing invalid-parameter handler. On failure, compare saved initial and
// current source iterators, erase predecessors of the fixed insertion position
// while advancing the saved source cursor, then rethrow. A rollback failure
// can supersede the original exception; this is not an all-or-nothing guard.
// Aliased/mutated source and destination graphs retain native unsafe behavior.
void insert_native_render_alias_range_004d26a0(void* actual_destination_owner,
    NativeRenderAliasIterator insertion_position_by_value,
    NativeRenderAliasIterator source_by_value, NativeRenderAliasIterator end_by_value,
    SizedStoragePool& actual_string_pool, const SingletonLifetimeCallbacks&);
// Actual owning-pool overload: every string operation uses the current 419CC0
// publication/gate/lifetime binding. Same native algorithm and exception limits.
void insert_native_render_alias_range_004d26a0(void* actual_destination_owner,
    NativeRenderAliasIterator insertion_position_by_value,
    NativeRenderAliasIterator source_by_value, NativeRenderAliasIterator end_by_value,
    ActualNativeStringPoolStorage& actual_string_pool, const SingletonLifetimeCallbacks&);

// Valid actual owner/node/string storage and the same shared allocation/handler
// domains are required. These new C++ entries use the owning host exception
// transport from native_alias_count_growth, not the original native throw ABI.


// Raw publication variant: fixed current CRT validation, throwing pool getter,
// and the same owning catch/rollback schedule. No external callback contract.
void insert_native_render_alias_range_004d26a0(void*, NativeRenderAliasIterator,
    NativeRenderAliasIterator, NativeRenderAliasIterator, NativeStringRawPoolContext&);

} // namespace bsp
