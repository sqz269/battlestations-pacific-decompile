#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native list count guards require MSVC Win32.
#endif

namespace bsp {

// Complete Source-level normal/error behavior of 00420140 and 0041FDD0:
// capture DWORD header+8 once; if uint32(3FFFFFFFh-current)<addition, construct
// and throw a genuine current-SDK std::length_error("list<T> too long").
// Otherwise store uint32(current+addition) at +8. No count store precedes error
// construction, no rollback of provider side effects, and no extra admission
// guard for prestates above 3FFFFFFFh. All arithmetic is unsigned DWORD wrap.
// Supply an existing readable/writable 0Ch header; +0h/+4h are untouched.
// No class, allocation, payload or outer-owner lifetime is invented.
//
// Original entries: __thiscall, ECX header, stacked uint32 addition, RET 4;
// each 147-byte body includes native FH3 and legacy exception construction.
// Source fastcall(header,unused_edx,addition) retains ECX header, stack addition
// and RET 4; the EDX slot is ignored. These entries use the current SDK
// exception/frame/handler identity; they do not reproduce native
// legacy string/object bytes, allocation schedule, throw metadata, FH3 frames,
// full register/stack-frame ABI, or identical native instructions. Both routines' Source
// error paths are implemented, not delegated to a success-only callback.
// Evidence: docs/GOAL15C_LIST_COUNT_GUARDS.md.
void __fastcall increment_native_list_count_00420140(
    void* actual_list_header, void* unused_edx, std::uint32_t addition);
void __fastcall increment_native_list_count_0041fdd0(
    void* actual_list_header, void* unused_edx, std::uint32_t addition);

} // namespace bsp
