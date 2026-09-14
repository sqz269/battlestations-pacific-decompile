#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer container lifetime requires MSVC Win32.
#endif

namespace bsp {

// Actual 0Ch header: pointer +0, signed count +4, signed capacity +8.
// These four complete bodies reserve raw records; they do not destroy owners.
// The renderer destructor reaches them only when a corresponding capacity is
// negative during resize-to-zero. Requested capacity is clamped to at least one.
// Allocation/copy/free precede publication of fresh pointer and capacity; count
// is not assigned. Copy reloads the current source pointer/count and uses the
// native per-row null-destination test, x87 FLD/FSTP and DWORD copy order.
//
// Reuses singleton_lifetime_allocate/free with equal native/host byte counts.
// No rollback is added on failure. Raw reachable extents and ownership must be
// valid for native wrapping 32-bit arithmetic; no overflow or malformed-header
// repair is performed. Uncopied allocation bytes stay untouched.
// Original ECX header, one signed stack DWORD, RET4. Ignored EDX is explicit in
// this source interface. Original private-frame aliases, hardware-fault unwind,
// static-CRT exception identity and game validation remain outside the contract.

// B228B0..B22933: stride10h; x87 +00,+04,+08; DWORD +0C.
void __fastcall reserve_native_renderer_records16_00b228b0(
    void* actual_header, std::uint32_t unused_edx, std::int32_t requested_capacity);

// B22940..B229C9: stride14h; x87 +00,+04,+08,+0C; DWORD +10.
void __fastcall reserve_native_renderer_records20_00b22940(
    void* actual_header, std::uint32_t unused_edx, std::int32_t requested_capacity);

// B229D0..B22A65: stride18h; x87 +00,+04,+08,+0C; DWORD +10,+14.
void __fastcall reserve_native_renderer_records24_00b229d0(
    void* actual_header, std::uint32_t unused_edx, std::int32_t requested_capacity);

// B22A70..B22B21: stride28h; DWORD +00,+14; x87 all other DWORDs.
void __fastcall reserve_native_renderer_records40_00b22a70(
    void* actual_header, std::uint32_t unused_edx, std::int32_t requested_capacity);

} // namespace bsp
