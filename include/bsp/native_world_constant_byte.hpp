#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native World constant byte requires MSVC Win32.
#endif

namespace bsp {

// Provisional descriptive name. Native [009035D0,009035D3) is MOV AL,1; RET.
// This ordinary C++ bool result models AL=true only. The native body also
// preserves upper EAX, flags and the other data registers; this Source interface
// does not promise those machine-state details or a full-EAX integer result.
// Explicit unused ECX receiver and EDX formal occupy both fastcall registers;
// there are no stacked arguments. No receiver/storage read, allocation,
// ownership or World table publication is performed. Slot meaning is unknown.
bool __fastcall native_world_constant_byte_009035d0(
    void* unused_ecx_receiver, std::uint32_t unused_edx) noexcept;

} // namespace bsp
