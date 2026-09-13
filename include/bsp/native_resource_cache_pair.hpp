#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Full B7F290 (161 bytes). Native ECX=actual 0Ch destination pair,
// EDX=raw resource, stack={owned name length,data} by value; EAX=destination,
// RET8. This C++ API exposes the actual input-header address so the native
// identity test and current-header unwind cleanup are retained. It neither
// starts a NativeString object nor takes an implicit C++ owner by value.
//
// Capture input length/data and resource before clearing destination+0/+4.
// Unless the two headers are identical, copy the captured input name through
// current destination fields; then store the resource at+8 without AddRef.
// Normal input cleanup uses captured fields. Initial copy failure cleans the
// CURRENT input header only; failure during normal input cleanup cleans the
// CURRENT completed destination name. Cleanup does not clear either header.
//
// Concrete actual pool publications are borrowed; getter failures propagate.
// No native STL pair/tree ABI, tree insertion, resource release, or rollback
// for partial name allocation is introduced. Valid raw Win32 storage and
// nonzero copy ranges are caller requirements. A zero-byte BF7680 call is
// omitted as in the existing raw-string API; overlap uses memmove.
// Nested cleanup exceptions and native FH3/SEH faults are outside the proven
// C++ exception domain. Evidence: docs/NATIVE_RESOURCE_CACHE_PAIR.md.
void* construct_native_resource_cache_pair_00b7f290(void* actual_destination,
    void* actual_resource, void* actual_owned_input_header,
    NativeStringRawPoolContext& actual_strings);

} // namespace bsp
