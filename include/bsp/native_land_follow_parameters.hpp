#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This recovered x87 kernel requires MSVC Win32.
#endif

namespace bsp {
// Native 009BE150: ECX=destination, stack=source, EAX=destination, RET4.
// The declared fastcall bridge reserves unused EDX so source remains on stack.
// Each block must expose at least A4h live bytes; exact self alias and disjoint
// spans are admitted. Partial overlap is outside the validated source domain.
// Forty ordered FLD/FSTP binary32 lanes at 00h..64h and 6Ch..A0h, with byte
// copies at 68h/69h between them. Bytes6Ah/6Bh are untouched. Self alias must
// still execute: signaling NaNs and ambient x87 status can change.
// Requires masked x87 exceptions, no pending unmasked exception, and a free
// x87 stack slot. Preserves the control word and existing stack values; status
// follows the actual loads/stores. No default block, pointer replacement,
// allocation, singleton binding, fault recovery or game lifetime is supplied.
void* __fastcall copy_native_land_follow_parameters_009be150(
    void* destination, const void* unused_edx, const void* source) noexcept;
} // namespace bsp

