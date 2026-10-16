#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native traffic DWORD resize requires MSVC Win32.
#endif

namespace bsp {

// Whole 00495AE0..00495B92:179 bytes/78 instructions. Original physical input:
// ECX borrowed raw16-byte header (opaque0,begin4,end8,capacityC), then two actual
// stack DWORDs {requested_count,value}. No incoming EDX input. Both normal
// returns use RET8. Source-only unused EDX keeps those two actual stack cells.
// The descriptive resize name is not a recovered class or Original declaration.
//
// Count is captured before diagnostics. Wrapped SUB/SAR2 and unsigned tests,
// returning real diagnostic calls, captured/reloaded header words and the
// original eight-byte uninitialized output area are preserved literally.
// Growth passes the address of the CURRENT actual value argument to whole
// 00493120; it does not snapshot that value. Shrink borrows the actual output
// area for whole00488FF0 and seeds only its second DWORD after diagnostics.
//
// The chosen uint32 result exposes physical EAX only: different paths leave a
// size, child insertion result, or the already-released internal output-area
// address. Neither Native caller consumes that result. No Original semantic
// return type or usable output-pointer lifetime is inferred.
//
// Genuine current SDK diagnostic/allocator/copy/free/owning host-EH service
// boundaries are inherited from the complete children. Original private CRT,
// FH3/RTTI/SEH/throw identity, C++ exception propagation through naked frames,
// full fault/flags/binary ABI and game/parent closure remain unproved. No added
// guard, null repair, RAII, exception conversion, noexcept or noreturn promise.
std::uint32_t __fastcall resize_native_traffic_dwords_00495ae0(
    void* raw_header, std::uint32_t source_unused_edx,
    std::uint32_t requested_count, std::uint32_t value);

} // namespace bsp
