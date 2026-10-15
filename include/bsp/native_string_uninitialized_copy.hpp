#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Complete 00431F40 argument-header destructor. Original ECX is unused,
// stack contains an actual8h {length,data} header, RET4. Capture data, then
// length+1 if nonnull; resolve the existing actual pool and return storage.
// No header clearing, object lifetime operation, or owner read is added.
void destroy_native_string_storage_argument_00431f40(void* ignored_owner,
    void* actual_header, NativeStringRawPoolContext& strings);

// Full normal loop0044D0C0 and catch0044D10A in the existing Source C++
// exception domain. Original ECX=first, EDX=last; stack destination, owner,
// unused destination duplicate, unused tag; RET10h/EAX=completed destination.
// Source carries the actual raw publication context explicitly. Captured raw
// addresses advance by8 using DWORD arithmetic. Construction uses accepted
// actual-header0044BCB0. On a C++ exception, destroy only completed elements,
// ascending, then rethrow; failed current element is deliberately excluded.
// Cleanup failure escapes immediately without cleaning later elements.
//
// The caller provides readable source headers, writable uninitialized output,
// valid element ranges and actual pool publications/lifetimes accepted by
// those services. No new overlap prohibition, validation, backing allocation,
// vector publication, or rollback is added. Original FH3/SEH catchability,
// binary unwind metadata, double-exception identity and stack/ABI identity
// are unproved; /EHsc C++ exceptions only. See the packet documentation.
void* copy_native_string_uninitialized_range_0044d0c0(const void* first,
    const void* last, void* destination, void* owner,
    NativeStringRawPoolContext& strings);

// Full00505AD0 ordinary dispatcher: original ECX=owner, stacked
// first/last/destination, RET0Ch, forwards EAX from the core. Native dead
// duplicate/tag words and private scratch layout are not Source ABI promises.
void* copy_native_string_uninitialized_dispatch_00505ad0(void* owner,
    const void* first, const void* last, void* destination,
    NativeStringRawPoolContext& strings);
} // namespace bsp
