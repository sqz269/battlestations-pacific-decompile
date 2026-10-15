#pragma once

namespace bsp {
// Complete normal admitted-range 0041C4D0..0041C4FA, cdecl first,last,dest,
// plain RET, EAX result from actual 00419330. Each 12-byte record has three
// binary32 float words.
// Dispatch six stack words first,last,dest,last,last,tag to the existing exact
// move kernel. Only tag's low BYTE is zeroed; upper 24 bits retain incoming ECX.
// The kernel ignores all three metadata words, so no tag preimage is invented.
// Retains forward overlap/x87 copy order and native signed divide12 end address.
// first==last does not read/write pointed storage and returns destination.
// Native addresses must reach last by +12 steps with every access admitted;
// actual source/destination storage and ambient FP state are borrowed unchanged.
void* __cdecl vector12_move_forward_dispatch_0041c4d0(
    const void* first, const void* last, void* destination) noexcept;

// Complete normal admitted-range 0041C580..0041C5A2, cdecl first,last,value,
// plain RET; EAX is final cursor (last on admitted ranges), including empty.
// Despite pseudocode's DWORD/void form, native copies two FLOAT words through
// x87 FLD/FSTP, quieting signaling NaNs under ambient control state. Each loop
// rereads value[0], writes destination[0], THEN rereads value[1] and writes it.
// No cached value pair or bitwise-copy policy: value may alias destination at
// any readable word offset, and first store may alter the second source word.
// Empty range touches no pointed storage, including value. Address increments
// are native32-bit ADD8/equality, not C++ pointer subtraction/arithmetic.
// Nonempty ranges must reach last by +8 steps with each source read/destination
// write admitted; stable mapped storage is required and invalid ranges are not
// repaired. No allocation, validation callback or host pointer model is added.
void* __cdecl pod8_range_fill_0041c580(
    void* first, const void* last, const void* value) noexcept;

// Original library-role Ghidra names are retained. These are callable source
// entries, not an installed binary replacement or proof of original exception
// ABI, FP trap behavior, packet-native differential or game equivalence.
} // namespace bsp
