#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene property record storage requires MSVC Win32.
#endif

namespace bsp {

// Whole [008EF140,008EF16D): 45 bytes, 14 instructions. The descriptive native
// name is a hypothesis. This is partial initialization of supplied raw storage,
// without class construction, dispatch binding, allocation or owning teardown.
inline constexpr std::size_t native_scene_property_record_type0_storage_bytes = 0x38;

// Physical ABI: ECX=root; ONE value DWORD at entry ESP+4; EAX=root; RET4.
// The unused EDX formal keeps value_bits on the stack. Incoming EDX is ignored.
// On return ECX=0 and EDX=value_bits; nonvolatile registers are untouched.
// Writes 37 bytes: [00,08), [0C,10), [18,28), [2C,2D), [30,38).
// Preserves 19 bytes: [08,0C), [10,18), [28,2C), [2D,30).
// +0 contains literal native DATA phase bits 00CE89D4, never Source dispatch;
// +4 is type0, +C stores value bits, +2C is byte1, +30/+34 are zero owner/ordinal.
// Supply fresh, unowned writable storage spanning at least 56 bytes. Value bits
// are not an owner argument and are never dereferenced. No DF or ES use exists.
void* __fastcall initialize_native_scene_property_record_type0_storage_008ef140(
    void* actual_storage_root, std::uint32_t unused_edx, std::uint32_t value_bits) noexcept;

} // namespace bsp
