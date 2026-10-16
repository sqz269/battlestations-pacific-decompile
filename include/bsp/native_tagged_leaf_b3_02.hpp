#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tagged B3 leaf entries require MSVC Win32.
#endif

namespace bsp {

// Operation names only: no Original owning class or member identity is known.
// Six readers consume ECX alone and finish with plain RET. A one-argument
// __fastcall boundary expresses this without inventing an EDX input.
// Backing must contain the specified readable four bytes; no alignment/class
// is imposed. Three constant entries consume no argument registers or stack.
std::uint32_t __fastcall native_tagged_b3_read_dword_84_00b343d0(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b34450(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b34460(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b34470(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b34480(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_00b344a0(const void* base) noexcept;

std::uint32_t __cdecl native_tagged_b3_return_0a_00b34500() noexcept;
std::uint32_t __cdecl native_tagged_b3_return_06_00b34510() noexcept;
std::uint32_t __cdecl native_tagged_b3_return_10_00b34520() noexcept;

} // namespace bsp
