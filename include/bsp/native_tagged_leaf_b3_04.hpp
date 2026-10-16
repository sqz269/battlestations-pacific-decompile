#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tagged B3 leaf entries require MSVC Win32.
#endif

namespace bsp {

// Complete operation boundaries, not recovered Original class/symbol names.
// Each entry consumes ECX only, no EDX/stack argument, and finishes plain RET.
// Address arithmetic and identity results are raw unsigned32 bits, preserving
// wraparound for every input. No pointer/object class or empty-case guard is
// imposed. The three scaled-address readers require eight readable bytes.
// Their complete Source retains normal-return value/read order, while the
// registered object changes volatile ECX/EDX/EFLAGS and fault register context.
// Those three entries are qualified new interfaces, not exact machine copies.
std::uint32_t __fastcall native_tagged_b3_scaled_address_00b3e370(const void* record) noexcept;
std::uint32_t __fastcall native_tagged_b3_scaled_address_00b3e4f0(const void* record) noexcept;
std::uint32_t __fastcall native_tagged_b3_scaled_address_00b3e670(const void* record) noexcept;
std::uint32_t __fastcall native_tagged_b3_identity_00b3ff40(std::uint32_t bits) noexcept;
std::uint32_t __fastcall native_tagged_b3_identity_00b3ff50(std::uint32_t bits) noexcept;
std::uint32_t __fastcall native_tagged_b3_identity_00b3ff60(std::uint32_t bits) noexcept;
std::uint32_t __fastcall native_tagged_b3_address_78_00b3ff90(std::uint32_t bits) noexcept;
// Only AL is a C++ result. Fixed-compiler whole-body proof retains upper EAX.
std::uint8_t __fastcall native_tagged_b3_read_byte_0e_00b3ffb0(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_34_00b3ffc0(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_170_00b3ffd0(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_190_00b3ffe0(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_08_00b3fff0(const void* base) noexcept;

} // namespace bsp
