#pragma once
#include "bsp/native_bit_cursor_numeric.hpp"
namespace bsp {
// Original ECX=cursor, six stack arguments, RET18h. Count is unsigned;
// buffers and 32-bit wrapping addresses retain native unchecked behavior.
// Scale is captured by value, then loaded/stored through x87 per element.
void write_native_numeric_float_array_00429790(NativeBitCursor*,const std::uint32_t*,std::uint32_t count,std::uint32_t zero_flag,std::uint32_t signed_flag,std::uint32_t scale_bits,std::uint32_t width,const NativeBitNumericContext&);
void read_native_numeric_float_array_004294f0(NativeBitCursor*,std::uint32_t*,std::uint32_t count,std::uint32_t zero_flag,std::uint32_t signed_flag,std::uint32_t scale_bits,std::uint32_t width,const NativeBitNumericContext&);
} // namespace bsp
