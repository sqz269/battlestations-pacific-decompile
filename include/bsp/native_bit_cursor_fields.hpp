#pragma once
#include "bsp/native_bit_cursor_write.hpp"
namespace bsp {
// Original ECX cursor; DWORD stack value/bits, RET8. Signed-byte callers pass
// zero-extended byte bits; the signed interpretation belongs to the reader.
void write_native_signed_byte_bits_00429010(NativeBitCursor*,std::uint32_t,std::uint32_t bits);
void write_native_unsigned_dword_bits_00429070(NativeBitCursor*,std::uint32_t,std::uint32_t bits);
// Scan through the actual terminator, then write length modulo256 and exactly
// that many leading bytes. No native length clamp or capacity check exists.
void write_native_bit_string_004290d0(NativeBitCursor*,const char*);
// Original two stack DWORD arguments, RET8. Capture both before writing64bits.
void write_native_u64_bits_00429180(NativeBitCursor*,std::uint32_t low,std::uint32_t high);
} // namespace bsp
