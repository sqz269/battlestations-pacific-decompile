#pragma once
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
struct NativeNodeStorage;

// Complete00711C60..00711E5B. ECX=actual1ACh part, RET. A null selected set
// is inert. Scan its checked8-byte records at+7C, copying record0's legacy SBO
// name and capturing record4's raw node. Only a first underscore at index4
// with the exact prefix "part" produces a10h-byte record: captured node at0
// and zero DWORDs at4/8/C.
// Append to the existing part+1A0 pointer array; preserve duplicates/order.
// Do not clear earlier entries or synthesize a hierarchy. A reserve failure
// after record allocation leaves that unpublished record allocated, as native.
// This new C++ ABI uses the existing source CRT/SBO/pointer-array domain;
// native FH3, corrupt storage, private-stack aliases and gameplay are unproved.
void build_native_unit_part_entries_00711c60(void* actual_part);

// Complete00B6F960..00B6F995. Native ECX=node, stack source8h pooled-string
// header, RET4. Self-assignment is inert. Resize actual node.name_54 through
// the current raw pool, then reload both headers before the overlap-capable
// CRT copy. No temporary name owner, reference retain or hierarchy change.
// A zero-byte copy alone is omitted after reading the native operands.
void assign_native_unit_part_node_name_00b6f960(NativeNodeStorage& actual_node,
    const NativeString& actual_name, NativeStringRawPoolContext&);

struct NativeUnitPartSelectedSetCallbacks {
    void* context;
    // Complete virtual slot0 operation: ECX=captured owner, no stack arguments.
    // Entry is the current native token read only AFTER the real decrement
    // reaches zero. It is not cast to process-callable code by the source.
    void (*dispatch_zero)(void*, std::uint32_t entry, void* captured_owner);
};
// Complete00711080..007110A8. ECX=actual selected-set pointer cell, RET. Null
// is inert; otherwise capture owner, InterlockedDecrement(owner+4), dispatch
// current slot0 only at zero, then clear the cell even if dispatch replaced it.
// Source callback exceptions escape before that final clear. Native exception
// ABI and the target selected-set destruction implementation remain separate.
void release_native_unit_part_selected_set_00711080(void* actual_cell,
    NativeUnitPartSelectedSetCallbacks);
} // namespace bsp
