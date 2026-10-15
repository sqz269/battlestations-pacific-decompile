#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Complete0044BC50 backward actual8h-header assignment. Native cdecl/plain
// RET/EAX advanced destination; first,last,destination_end are consumed.
// Known dispatchers supply three additional ignored DWORDs, retained here;
// Source additionally borrows the actual publication/pool context. Each step
// decrements source and destination by8 BEFORE identity check and assignment.
// Empty returns destination_end without any pointed read. Captured pointer
// values use DWORD wrapping arithmetic; there is no overlap/range validation.
void* __cdecl copy_native_string_header_range_backward_0044bc50(
    const void* first, const void* last, void* destination_end,
    std::uint32_t unused4, std::uint32_t unused5, std::uint32_t unused6,
    NativeStringRawPoolContext& strings);

// Complete0044C770: cdecl three native pointers, plainRET, no semantic result.
// Capture value HEADER ADDRESS, not its fields. Walk destination ascending;
// each element reuses the accepted actual-header assignment schedule with
// CURRENT value length/data and post-resize destination/value field reloads.
// Identical header addresses skip all reads. Empty range touches no value.
void __cdecl fill_native_string_header_range_0044c770(void* first,
    const void* last, const void* value, NativeStringRawPoolContext& strings);

// Complete0044D850 ordinary cdecl3ptr dispatcher, plainRET/EAX independently
// computed destination_end-SAR32(last-first,3)*8, all DWORD wrapping. Child
// EAX is deliberately ignored. Native dead tag low bytes are zeroed; Source
// supplies zero for their unobserved high bytes and retains duplicate dstEnd.
void* __cdecl copy_native_string_backward_dispatch_0044d850(const void* first,
    const void* last, void* destination_end, NativeStringRawPoolContext& strings);

// Complete0044F170: original ECX owner, stack first,last,destination, RET0Ch,
// EAX completed destination. Its consumed-input/body schedule equals accepted
// 00505AD0, so the actual uninitialized-copy service and prefix cleanup are
// reused. No duplicate loop, new allocator or replacement callback is added.
void* copy_native_string_uninitialized_dispatch_0044f170(void* owner,
    const void* first, const void* last, void* destination,
    NativeStringRawPoolContext& strings);

// Valid mapped taken storage, current pool/manager publications and ownership
// must satisfy the accepted child services. No host string/vector overlay,
// guards, local rollback or cleanup are introduced. Source CPP exceptions,
// currentCRT overlap-copy and omitted zero-byte-copy policy are inherited.
// Extra context and private frames differ from original ABI/FH3/SEH/fault and
// register/flag identity; no native/gameplay compatibility is asserted.
} // namespace bsp
