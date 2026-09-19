#pragma once
#include "bsp/native_bit_cursor_read.hpp"
#include "bsp/native_string.hpp"
namespace bsp {
// Actual eight-byte header: DWORD length, char* data. Original ECX=cursor,
// one stack header pointer, RET4. These are explicit source interfaces.
// Writer captures only the low length byte, writes it, then reloads data.
// Null data uses the actual E17669 backing, which must cover the wire length.
void write_native_owned_string_00429ac0(NativeBitCursor*,const void* header,const char* actual_fallback_00e17669);
// Consumes the entire byte-counted payload into a 256-byte stack buffer,
// stores only through its first NUL, and preserves the raw resize quirks.
void read_native_owned_string_00429f20(NativeBitCursor*,void* header,NativeStringRawPoolContext&);
} // namespace bsp
