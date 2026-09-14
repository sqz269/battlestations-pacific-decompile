#pragma once

namespace bsp {
struct NativeRawNameReaderContext;

// Complete in-place BEA250 source schedule for valid raw Win32 storage.
// Native ECX is writable 24h node storage, stack parent is an initialized
// raw 24h node, EAX returns the node, RET4. This context-bearing interface
// uses the actual reader and string-pool providers, not the native call ABI.
// Reader path entries already designate initialized writable raw8h headers.
void* construct_native_raw_child_node_00bea250(void* actual_node24h,
    void* actual_parent24h, NativeRawNameReaderContext& context);
} // namespace bsp
