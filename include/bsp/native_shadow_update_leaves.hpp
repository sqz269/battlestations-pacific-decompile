#pragma once

#include <cstdint>

namespace bsp {

// Full 00B7AAE0 current-word read through an explicit borrowed-cell C++ ABI.
// Bind LightTypeBootstrapStorage::directional_0109019c.own_id (also exposed by
// GameNativeTypeStorage::light_types()). Do not initialize, cache or substitute
// the cell. Native had no inputs and returned [0109019C] in EAX; this interface
// does not claim that original no-argument ABI or six-byte instruction identity.
std::uint32_t get_native_override_context_word_00b7aae0(
    const volatile std::uint32_t& actual_0109019c) noexcept;

// Full 00B400C0, 13 bytes. Actual receiver ECX; only AL is loaded from the first
// public DWORD at entryESP+4, then stored to receiver+13Ch; RET4. Unused EDX
// preserves that original stack slot in the source fastcall interface. The
// writable actual receiver is caller-owned; no bool normalization or validation.
void __fastcall set_native_material_effect_byte_00b400c0(
    void* actual_effect, void* unused_edx, std::uint32_t public_word);

// Full 00B40820, 22 bytes. Actual receiver ECX; index at entryESP+4, binary32
// bits at entryESP+8; MOVSS copies to receiver+index*4+140h and RET8. Raw DWORD
// value_bits avoids float argument conversion. Native x86 address arithmetic
// and unchecked index are preserved; callers provide valid actual storage.
void __fastcall set_native_material_effect_float_bits_00b40820(
    void* actual_effect, void* unused_edx,
    std::uint32_t index, std::uint32_t value_bits);

} // namespace bsp
