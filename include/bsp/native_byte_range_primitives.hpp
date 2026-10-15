#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native byte-range primitives require MSVC Win32.
#endif

namespace bsp {
// Complete 0041F450. Stack(first,count,value), RET0Ch; EAX first+count
// modulo2^32. Count zero does not dereference value. Each iteration reloads
// *value before its byte store, including when value aliases the output.
void* __stdcall fill_n_native_byte_range_0041f450(
    void* first, std::uint32_t count, const void* value);

// Complete 0041C630. Cdecl(first,last,value), plain RET; no source result is
// claimed. Equal endpoints do not dereference value; every other iteration
// reloads *value. Address increments and endpoint comparison use DWORDs.
void __cdecl fill_native_byte_range_0041c630(
    void* first, void* last, const void* value);

// Complete 0041C650. Cdecl(first,last,destination_end), plain RET; EAX is
// destination_end - uint32(last-first), including when the copy is skipped.
// Calls current Source memmove_s(result,n,first,n) only for 0<n<80000000h.
void* __cdecl copy_backward_native_byte_range_0041c650(
    void* first, void* last, void* destination_end);

// Complete 0041E630. Stack(first,last,destination), RET0Ch; EAX is
// destination+uint32(last-first). Calls current Source memmove_s(dst,n,first,n)
// whenever n!=0, including n with bit31 set. Both copies ignore its result;
// current CRT invalid-handler/errno policy is retained, not native CRT identity.
// All address arithmetic wraps as Win32 DWORDs. These new Source entries
// retain the observed argument/return shapes, not native binary/SEH identity.
// No admission guards or alternate provider is installed. Any accessed spans
// must be valid when reached. Evidence: docs/GOAL15B_BYTE_RANGE_PRIMITIVES.md.
void* __stdcall copy_native_byte_range_0041e630(
    void* first, void* last, void* destination);
} // namespace bsp
