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

// Full 00436430, native ECX=header (__fastcall), RET. Capture a nonnull buffer
// and current 2*length+2 byte size, then release. No header writes, even if the
// release callback changes it. A null pointer skips the length read entirely.
void destroy_native_wide_string_header_00436430(void* actual_header,
    NativeStringStorage& storage) noexcept;

} // namespace bsp
