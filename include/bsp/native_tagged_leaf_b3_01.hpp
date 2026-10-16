#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tagged B3 leaf entries require MSVC Win32.
#endif

namespace bsp {

// Original entries 00B34300..00B34370 consume only ECX. These one-argument
// __fastcall boundaries preserve that contract without inventing an EDX input.
// Names describe observed operations; no original class or symbol is recovered.
// Each load requires readable backing at its offset. Address entries operate
// on 32-bit address bits, retaining LEA's modulo-32 arithmetic for every input.
// No validation, ownership, global state, or calls exist.
std::uint32_t __fastcall native_tagged_b3_read_dword_00b34300(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b34310(const void* base) noexcept;
// Only AL is defined by the Original and Source entry; upper EAX is unspecified.
std::uint8_t __fastcall native_tagged_b3_read_byte_0c_00b34320(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_10_00b34330(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_address_04_00b34340(std::uint32_t base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_20_00b34350(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_14_00b34360(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_address_18_00b34370(std::uint32_t base) noexcept;

} // namespace bsp
