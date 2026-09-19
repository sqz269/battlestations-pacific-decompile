#pragma once
#include "bsp/native_bit_cursor_read.hpp"

namespace bsp {
// ECX is the raw 10h cursor for these writers. Backing must be writable and
// include the native carry byte: every full byte also stores the following
// byte, including a zero carry for an aligned write. No capacity clamp.
void write_native_bits_00428f50(NativeBitCursor*,const void*,std::uint32_t bits);
// Original stack DWORD value/bits, RET8. Intended widths are byte/WORD; the
// helper reads the low bytes of the actual argument. New source interfaces.
void write_native_byte_bits_00428ff0(NativeBitCursor*,std::uint32_t value,std::uint32_t bits);
void write_native_bool_bit_004290b0(NativeBitCursor*,std::uint32_t value);
void write_native_word_bits_00429120(NativeBitCursor*,std::uint32_t value,std::uint32_t bits);
} // namespace bsp
