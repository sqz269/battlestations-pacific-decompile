#pragma once
#include <cstddef>
#include <cstdint>

namespace bsp {
// Actual 10h cursor embedded at +4 in the 18h session bitstream. No defaults:
// callers provide the original backing range, current byte and bit offset.
struct NativeBitCursor {
    const std::uint8_t* base_00;
    std::uint32_t length_04;
    const std::uint8_t* current_08;
    std::int32_t bit_0c;
};
static_assert(sizeof(NativeBitCursor)==0x10);
static_assert(offsetof(NativeBitCursor,current_08)==8);
static_assert(offsetof(NativeBitCursor,bit_0c)==0xc);

// Original ECX=cursor. Raw/typed readers take destination,bits and RET8;
// rewind takes bits/RET4. These C++ signatures are new source interfaces.
// Keep native unchecked range behavior: a nonaligned whole byte can read the
// following byte without consulting length. Partial reads alone check it.
void rewind_native_bits_00428b80(NativeBitCursor*,std::uint32_t bits);
void read_native_bits_00428bb0(NativeBitCursor*,void*,std::uint32_t bits);
void read_native_u8_bits_00428c70(NativeBitCursor*,std::uint8_t*,std::uint32_t);
void read_native_i8_bits_00428c80(NativeBitCursor*,std::int8_t*,std::uint32_t);
void read_native_u16_bits_00428cb0(NativeBitCursor*,std::uint16_t*,std::uint32_t);
void read_native_i16_bits_00428cd0(NativeBitCursor*,std::int16_t*,std::uint32_t);
void read_native_u32_bits_00428d10(NativeBitCursor*,std::uint32_t*,std::uint32_t);
void read_native_i32_bits_00428d30(NativeBitCursor*,std::int32_t*,std::uint32_t);
// ECX=cursor, one stack output/RET4. Boolean reads exactly one bit.
void read_native_bool_bit_00428d70(NativeBitCursor*,bool*);
// An unsigned byte length precedes the characters. Allocating form publishes
// the allocation before reading characters and reloads *out for termination.
// Uses the existing malloc/new-handler allocation boundary; free with
// singleton_lifetime_free. Original allocator identity/EH is not established.
void allocate_native_bit_string_00428da0(NativeBitCursor*,char**);
void read_native_bit_string_00428df0(NativeBitCursor*,char*);
// E30 is a second unsigned WORD wrapper, not a distinct arithmetic conversion.
void read_native_word_bits_00428e30(NativeBitCursor*,std::uint16_t*,std::uint32_t);
// Original stack output/count/bits, RET0C. Count is an unsigned loop count.
void read_native_word_array_00428e50(NativeBitCursor*,std::uint16_t*,std::uint32_t count,std::uint32_t bits);
// Original one output/RET4: first read eight local bytes, then publish two words.
void read_native_u64_bits_00428e90(NativeBitCursor*,std::uint32_t* two_words);
} // namespace bsp
