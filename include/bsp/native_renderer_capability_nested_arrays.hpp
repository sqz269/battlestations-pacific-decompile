#pragma once

#include <cstdint>

namespace bsp {

// MSVC Win32 raw arrays: each actual header is three DWORDs {data, signed
// count, signed capacity}. Inner elements are12 bytes: format DWORD, four
// flag bytes, then one payload DWORD. Outer elements are the inner headers.
// Backing allocations use the existing singleton_lifetime shared heap.
// All reached raw accesses require valid extents/lifetimes for the native
// wrapping arithmetic, including any supported header/buffer aliases.
// No malformed-storage, overflow, self-assignment or ownership repair occurs.
//
// Original ECX header, one callee argument DWORD, RET4. The explicit ignored
// EDX argument is a new source interface. These are not native caller/SEH ABI
// replacements. Source/target arguments are actual callee slots, not pointers
// directly into a camera/renderer owner or typed container projection.

// B23120[96]: grow zeros only element+0 and the four bytes+4..7. Element+8 is
// untouched. Shrink decrements current count, then publishes requested count.
void __fastcall resize_native_capability_records_00b23120(
    void* actual_header, std::uint32_t unused_edx, std::int32_t requested_count);

// B25E60[133]: first resize destination0; then capture source and unconditionally
// reserve its current count (empty source can allocate one element). Preserve
// the current source-row pointer across destination reserve. The actual source
// argument slot becomes the loop index after source capture. Returns destination.
void* __fastcall copy_native_capability_records_00b25e60(
    void* actual_destination, std::uint32_t unused_edx, void* actual_source);

// B29D40[248]: deep-copy current inner headers, then resize/free current old
// inner data and current outer data before publication. The actual request slot
// is clamped in place and reloaded after an entered old-inner destruction loop.
// A fixed C++ core uses explicit volatile saved state/locals and the exact no-op
// unwind specialization. Copy failure frees neither fresh outer nor completed
// inner allocations. It propagates C++ exceptions without added rollback.
// Original hardware/SEH faults and aliases into the native private EH frame are
// outside this new source frame/exception ABI; the actual request slot is retained.
void __fastcall reserve_native_capability_headers_00b29d40(
    void* actual_header, std::uint32_t unused_edx, std::int32_t requested_capacity);

// B2AE20[116]: grow initializes three zero words per header. Shrink publishes
// the decrement before current child resize0 and current child-data free.
void __fastcall resize_native_capability_headers_00b2ae20(
    void* actual_header, std::uint32_t unused_edx, std::int32_t requested_count);

} // namespace bsp
