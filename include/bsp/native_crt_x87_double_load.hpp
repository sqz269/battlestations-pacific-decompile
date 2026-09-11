#pragma once

#include <cstdint>

namespace bsp {

// Complete original CRT __fload_withFB at 00C083D5..00C08418 (67 bytes).
// ASSEMBLY CALLERS ONLY. This declaration names a register/x87 entry; it is
// not an ordinary safe C++ call or a C++ floating-point return interface.
// EDX addresses eight currently readable bytes. ECX has no input meaning;
// the exceptional path clobbers it. EDX and nonvolatile registers survive.
// On normal return exactly one value has been pushed onto the x87 stack.
// The caller must provide x87 stack space, consume ST0, and consume any needed
// EFLAGS before subsequent instructions overwrite them. Current x87 control,
// status and exception behavior apply; the function neither saves nor resets
// the floating-point environment and does not promise return after a fault.
// Finite route: EAX is the first high DWORD masked with 7FF00000h; EFLAGS
// come from CMP EAX,7FF00000h. Exceptional route: EAX is a final current high
// DWORD reload; EFLAGS come from TEST EAX,0 (ZF=1, CF=OF=SF=0, PF=1, AF
// undefined). Input high/low words are reread in the original order, without
// an atomic snapshot. The exceptional route uses a ten-byte stack temporary.
// The unused ECX parameter preserves EDX placement under this new MSVC Win32
// name; it does not make this a drop-in replacement at the original address.
std::uint32_t __fastcall load_native_crt_double_x87_00c083d5(
    void* unused_ecx, const void* actual_double);

} // namespace bsp
