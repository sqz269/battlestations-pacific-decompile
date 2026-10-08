#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene property record byte storage requires MSVC Win32.
#endif

namespace bsp {

// Whole [008EF1F0,008EF221): 49 bytes, 14 instructions. Descriptive native
// naming is a hypothesis. Partial supplied storage only; no class construction,
// Source dispatch, allocation or owning teardown is established here.
inline constexpr std::size_t native_scene_property_record_type3_byte_storage_bytes = 0x38;

// ECX=root; low byte at entry ESP+4; EAX=root; ECX=0; EDX unchanged; RET4.
// Unused EDX formal keeps value_bits in a rounded DWORD stack slot. Incoming
// EDX is neither read nor written. Upper three stack argument bytes are ignored.
// All nonvolatiles and DF/ES are untouched. Raw uint8 value, no Boolean conversion.
// Writes 34 bytes: [00,08), [0C,0D), [18,28), [2C,2D), [30,38).
// Preserves 22 bytes: [08,0C), [0D,18), [28,2C), [2D,30).
// Payload +C is stored BEFORE literal DATA phase CE89D4 and tag3. +2C is byte1;
// +30/+34 are zero owner/ordinal. Value is not owner and is never dereferenced.
// Supply fresh/unowned writable storage spanning at least 56 bytes. Phase bits
// are original DATA identity only and must never be dispatched as Source here.
void* __fastcall initialize_native_scene_property_record_type3_byte_storage_008ef1f0(
    void* actual_storage_root, std::uint32_t unused_edx, std::uint8_t value_bits) noexcept;

} // namespace bsp
