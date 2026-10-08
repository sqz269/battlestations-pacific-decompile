#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene property record type-4 storage requires MSVC Win32.
#endif

namespace bsp {

// Whole [008EF230,008EF268): 56 bytes, 16 instructions. Descriptive names
// are hypotheses. This partially initializes supplied raw storage without
// establishing a native declaration, enum lookup, class or owning lifetime.
inline constexpr std::size_t native_scene_property_record_type4_storage_bytes = 0x38;

// Supply actual fresh/unowned writable 56-byte storage, disjoint from the
// active call frame. The caller retains allocation ownership. Do not overwrite
// a live owning property object with this operation.
// ECX=root; incoming EDX unused; [entry ESP+4]=identity, [ESP+8]=value; RET8.
// Both arguments are opaque DWORDs: neither is dereferenced or converted.
// EAX=root, ECX=0, EDX=value; EBX/ESI/EDI/EBP and DF are unchanged.
// Writes 41 bytes: [00,08), [0C,10), [18,2D), [30,38).
// Preserves 15 bytes: [08,0C), [10,18), [2D,30).
// The first word is read before any store; the second is read after the +28
// store. The literal +00 word 00CE89D4 is native phase identity only and must
// never be dispatched as a Source vtable here. Tag +04 is the DWORD 4.
// Final XOR flags: CF=OF=SF=0, ZF=PF=1; AF is undefined. No target SSE/x87,
// MXCSR or numeric operation occurs. Native clone/allocator/EH and declaration
// ownership, parsing and lookup are outside this raw storage contract.
void* __fastcall construct_native_scene_property_record_type4_storage_008ef230(
    void* actual_record_ecx, void* unused_edx,
    std::uint32_t declaration_identity_bits, std::uint32_t value_bits) noexcept;

} // namespace bsp
