#pragma once
#include "bsp/native_bit_cursor_write.hpp"

namespace bsp {
// Borrow the actual process constants and mutable CRT conversion selector.
// No default state: their storage must outlive calls and any message profile.
struct NativeBitNumericContext {
    const volatile float& zero_00d7a218;
    const volatile float& minus_one_00d7a260;
    const volatile float& plus_one_00d7a24c;
    const volatile float& uint32_bias_00ce3978;
    const volatile double& unscaled_00d7a278;
    const volatile double& half_00d7a280;
    const volatile std::uint32_t& conversion_mode_0109eea4;
};
// Original ECX=cursor, five stack arguments, RET14h. These source interfaces
// add explicit context. Float values/scales use raw bits to retain sNaNs in the
// unscaled 32-bit codec path. Typed callers may themselves perform x87 loads.
// Native callers provide widths <=32 and sufficient input/output backing.
void write_native_numeric_float_004295c0(NativeBitCursor*,std::uint32_t value_bits,
    std::uint32_t zero_flag,std::uint32_t signed_flag,std::uint32_t scale_bits,
    std::uint32_t bits,const NativeBitNumericContext&);
void read_native_numeric_float_004293f0(NativeBitCursor*,std::uint32_t* value_bits,
    std::uint32_t zero_flag,std::uint32_t signed_flag,std::uint32_t scale_bits,
    std::uint32_t bits,const NativeBitNumericContext&);
void write_native_signed_dword_bits_00429090(NativeBitCursor*,std::uint32_t value,std::uint32_t bits);
} // namespace bsp
