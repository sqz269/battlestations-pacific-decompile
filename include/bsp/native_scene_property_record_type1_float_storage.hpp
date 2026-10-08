#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene property record float storage requires MSVC Win32.
#endif

namespace bsp {

// Whole [008EF170,008EF1A5): 53 bytes, 15 instructions. Descriptive naming
// is a hypothesis. This partially initializes supplied raw storage; it does
// not establish native class construction, phase dispatch or owning lifetime.
inline constexpr std::size_t native_scene_property_record_type1_float_storage_bytes = 0x38;

// Supply actual fresh/unowned writable 56-byte storage, disjoint from the
// active call frame. The caller retains raw allocation ownership. Do not use
// this operation to overwrite a live owning property object.
// ECX=root; incoming EDX unused; exact DWORD payload at entry ESP+4; RET4.
// EAX=root, ECX=0, EDX=1; EBX/ESI/EDI/EBP and DF are unchanged.
// Writes 37 bytes: [00,08), [0C,10), [18,28), [2C,2D), [30,38).
// Preserves 19 bytes: [08,0C), [10,18), [28,2C), [2D,30).
// Legacy MOVSS copies all raw payload bits to +0C and XMM0.low32, clearing
// XMM0.upper96. No float conversion, NaN quieting, x87 or MXCSR mutation.
// XMM1..7 are unchanged. No YMM/ZMM guarantee is made. Final XOR flags are
// CF=OF=SF=0, ZF=PF=1; AF is undefined. The literal +00 word 00CE89D4 is
// native phase identity only; never dispatch it as a Source vtable here.
// The real clone caller's separate FLD/FSTP conversion, native allocator/EH,
// class/pool/destructor ownership and whole-clone behavior are outside scope.
void* __fastcall construct_native_scene_property_record_type1_float_storage_008ef170(
    void* actual_record_ecx, void* unused_edx, std::uint32_t payload_bits) noexcept;

} // namespace bsp
