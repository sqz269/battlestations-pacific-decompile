#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tagged B3 leaf entries require MSVC Win32.
#endif

namespace bsp {

// Complete one-ECX-input word readers; no EDX/stack input is invented. Names
// describe operations. Some entries have known texture/surface vtable-region
// anchors, but no Original class layout or field meaning is asserted here.
// Backing must provide four readable bytes at the given offset; memcpy in
// Source avoids imposing an alignment, alias type, or speculative class.
std::uint32_t __fastcall native_tagged_b3_read_dword_18_00b3cd00(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_30_00b3ce90(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_24_00b3d090(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_28_00b3d0a0(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_2c_00b3d0b0(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b3d100(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b3d380(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_00b3d390(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b3d3c0(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_00b3d3d0(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_04_00b3d400(const void* base) noexcept;
std::uint32_t __fastcall native_tagged_b3_read_dword_00b3d410(const void* base) noexcept;

} // namespace bsp
