#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw DWORD range movement requires MSVC Win32.
#endif

namespace bsp {
// Complete 00467D90 [50 bytes, 21 instructions]. Native cdecl stack inputs
// first,last,destination; plain RET; EAX returns a pre-call captured address.
// Read last then first, subtract modulo32, arithmetic-shift the raw difference
// by two, capture destination, multiply the shifted bits by four modulo32,
// then capture destination+bytes. Only signed-positive shifted distance calls
// actual current SDK memmove_s(destination,bytes,first,bytes); ignore status.
// Zero/negative paths still return the wrapped/rounded captured address.
void* __cdecl move_native_scene_dword_range_00467d90(
    const void* first, const void* last, void* destination);

// New ordinary Source interface with raw DWORD address arithmetic. No nominal
// pointee/container type, alignment/order guard, callback or stub is imposed.
// All three native argument values and the result are captured before the CRT
// can report and return. Source argument/private-stack coincidence and original
// static-CRT errno/handler/register/SEH/binary ABI identity remain unproved.
// Reached backing must remain valid for the actual current SDK provider.
} // namespace bsp
