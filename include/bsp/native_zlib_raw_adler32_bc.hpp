#pragma once

#include <cstdint>

// New interface for the observed ECX seed, EDX bytes, one actual stack DWORD
// and RET4 binding. Original formal types/compiler flags remain unproved.
extern "C" std::uint32_t __fastcall bsp_native_zlib_raw_adler32_00bcd720(
    std::uint32_t seed, const void* bytes, std::uint32_t length) noexcept;
