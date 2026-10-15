#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw pooled-string range assignment requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete 0048DA50..0048DAA1 over actual8h {length0,data4} string headers.
// Native cdecl/plain RET/EAX advanced destination; six physical DWORD inputs,
// only first/last/destination consumed. The final three words stay explicit;
// this Source entry additionally borrows the actual string-pool context.
// Walk by pointer equality and wrapping DWORD +8, capturing range addresses.
// Identical source/destination headers skip ALL field reads, resize and copy.
// Otherwise resize destination from source length with preserve=true through
// the concrete raw-pool0041DD40 body; afterward recheck CURRENT source length.
// Nonzero source length reads CURRENT destination length, source data, then
// destination data, and overlap-copies exactly that destination length.
// No terminator is copied; only resize's existing schedule may write one.
void* __cdecl assign_native_string_header_range_0048da50(const void* first,
    const void* last, void* destination, std::uint32_t unused_caller_word,
    std::uint32_t unused_dispatch_word_a, std::uint32_t unused_dispatch_word_b,
    NativeStringRawPoolContext& strings);

// Complete 0048F400..0048F444. Native cdecl has three stacked pointers and
// plain RET/EAX result. Source adds the borrowed actual-pool context. Supply
// all six physical child words (three pointers, duplicate destination, two
// ignored tags), then independently compute destination+SAR32(last-first,3)*8
// with DWORD wrap; do not substitute the child's EAX. Source zeroes both dead
// dispatch words rather than reading indeterminate native private high bytes.
void* __cdecl assign_native_string_range_dispatch_0048f400(const void* first,
    const void* last, void* destination, NativeStringRawPoolContext& strings);

// Existing actual419CC0/BD1120/BD1510 providers are reused directly. No new
// allocator/copy callbacks, header overlay, staging strings or owner are added.
// Copies use the accepted BF7680 overlap-safe current-CRT memmove boundary.
// Its zero-byte call is omitted after retaining the outer header reads, like
// the existing raw resize service. Valid taken raw backing/ownership is needed.
// Exceptions propagate with completed prefix changes intact; no new cleanup,
// rollback, noexcept/noreturn or Source-to-native EH conversion is introduced.
// Private-stack aliases, exact original ABI/CRT/fault identity and gameplay are
// outside these Source interfaces. The earlier storage fragment remains separate.

} // namespace bsp
