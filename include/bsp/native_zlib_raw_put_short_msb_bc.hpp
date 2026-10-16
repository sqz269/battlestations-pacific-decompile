#pragma once

#include <cstdint>

// Chosen returning Source interface. Original putShortMSB consumes EAX backing
// and ECX raw value, with no explicit stack words. This is not its recovered ABI.
// Raw backing/output/current-stack aliases are admitted; return is not guaranteed.
extern "C" void __fastcall bsp_native_zlib_put_short_msb_00bcb1f0(
    void* backing, std::uint32_t value) noexcept;
