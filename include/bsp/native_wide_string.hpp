#pragma once

#include "bsp/native_string.hpp"

namespace bsp {

// Operate on the caller's actual eight-byte Win32 header: uint32 length at +0,
// pointer to 16-bit code units at +4. No temporary string or allocator is held
// in the header. The caller supplies its existing NativeStringStorage owner;
// ActualNativeStringPoolStorage binds the actual native owner; PooledStringStorage
// is the earlier semantic pool projection.
// These C++ entry points do not reproduce the original calling conventions.
// Evidence, alias ordering and remaining boundaries: docs/NATIVE_WIDE_STRING.md.

// Full 004C5E60, native thiscall(header, const char*), RET 4, EAX=header.
// Clear length, then pointer, before reading source. Resize with preserve=true;
// zero-extend each source byte through NUL into the captured destination.
// Fresh storage is expected: an existing buffer is abandoned without release.
// Source must be readable through NUL; no locale or encoding conversion occurs.
void* construct_native_wide_string_header_004c5e60(void* actual_header,
    const char* source, NativeStringStorage& storage);

// Full 004C53E0, native thiscall(header, uint32 length, byte preserve), RET 8.
// Equal length does nothing. Zero length releases then clears pointer/length.
// Otherwise allocate 2*length+2 bytes, optionally memcpy 2*min(old,new) bytes,
// release the current old buffer, publish pointer/length, then store a WORD NUL.
// Header reads after storage callbacks use the actual current header. All size
// and address arithmetic wraps at 32 bits. No allocation-failure/overflow guard.
// Zero-byte memcpy is omitted under the same host policy as NativeString.
void resize_native_wide_string_header_004c53e0(void* actual_header,
    NativeStringStorage& storage, std::uint32_t length, bool preserve);

// Same actual-header body through the caller's SAME AA8/AA4/AA0 raw cells.
// Resolve419CC0 before allocation and EVERY nonnull return (large/disabled
// included); capture return operands before its getter. Getter exceptions
// propagate, unlike the pre-existing noexcept NativeStringStorage::release
// boundary. No caching, adapter, header/object cast, rollback or local cleanup.
// Raw overlap uses memmove; zero-byte copy call is omitted after operand reads.
// Source/CRT/null/fault/FH3/SEH limits remain; no Original ABI/startup claim.
void resize_native_wide_string_header_004c53e0(void* actual_header,
    NativeStringRawPoolContext& context, std::uint32_t length, bool preserve);

// Full 00436430, native ECX=header (__fastcall), RET. Capture a nonnull buffer
// and current 2*length+2 byte size, then release. No header writes, even if the
// release callback changes it. A null pointer skips the length read entirely.
void destroy_native_wide_string_header_00436430(void* actual_header,
    NativeStringStorage& storage) noexcept;

// Same header schedule through the actual raw pool/manager publications.
// Capture the buffer and wrapping byte size before 00419CC0, then return via
// 00BD1510. Getter exceptions propagate; no header field is cleared or reread.
void destroy_native_wide_string_header_00436430(void* actual_header,
    NativeStringRawPoolContext& context);

// Full 004C8DD0[72B/27ins], native ECX=dst, stack src, EAX=SAME dst, RET4.
// Valid actual8B headers/backing and the SAME retained storage domain are
// required; production borrows existing GameNativeStringProcess::strings().
// Clear dst length then data even for exact self identity, abandoning any old
// backing without release. Otherwise resize using captured src length and
// preserve1, then reread current src length. Only nonzero captures current src
// data THEN current dst data and streams WORDs through and including NUL.
// No typed string cast, shared backing ownership, counted copy or validation.
// No local cleanup/catch/rollback; inherited Source resize/storage failure and
// noexcept limits remain, not Native FH3/private-ABI compatibility. Caller
// retains valid headers, reached backing and storage through call/unwind.
void* copy_construct_native_wide_string_header_004c8dd0(void* actual_header,
    const void* actual_source, NativeStringStorage& storage);

} // namespace bsp

namespace bsp {

// Full 004C5E20[57B/24 starts], native ECX=dst, stack src, EAX=SAME dst, RET4.
// Actual DWORD-aligned 8B headers/backing and the SAME retained string storage
// are required; production borrows GameNativeStringProcess::strings().
// Exact self identity returns before ANY header read, without initialization.
// Otherwise resize using captured source length/preserve1, then reread CURRENT
// source length. Only nonzero reads CURRENT source data THEN destination data
// and streams WORDs through and including NUL, with no count bound or memcpy.
// No new callback/owner, local cleanup/rollback or retry. Inherit real resize's
// wrapping arithmetic, allocation/alias and Source release-noexcept limits.
// Existing typed API/algorithm stays separate. This is a new Source ABI, not
// Native thiscall/FH3/private-CRT identity or raw Text rebuild/layout closure.
void* assign_native_wide_string_header_004c5e20(void* actual_header,
    const void* actual_source, NativeStringStorage& storage);

} // namespace bsp
