#pragma once

#include <cstdint>

// New raw-storage interfaces. No original class, field type, container owner or
// calling-convention spelling is inferred from the sole physical ECX input.
// Reads require the addressed four-byte field(s) to be accessible storage.
extern "C" std::uint32_t __fastcall bsp_tagged_b6_16e_00b663f0(const void*) noexcept;
extern "C" std::int32_t __fastcall bsp_tagged_b6_16e_00b6bdd0(const void*) noexcept;
extern "C" std::uint32_t __fastcall bsp_tagged_b6_16e_00b6d980(const void*) noexcept;
// Address arithmetic is modulo 2^32 over every input bit pattern; no memory read.
extern "C" std::uint32_t __fastcall bsp_tagged_b6_16e_00b6fc60(std::uint32_t) noexcept;
