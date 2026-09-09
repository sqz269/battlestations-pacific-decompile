#pragma once
#include "bsp/memory_stream.hpp"

namespace bsp {
// Null-actual-pointer projections used by native font loader 00ad4c30.
// Original ABI: ECX stream, optional actual-count pointer stack, RET4.
// Reused argument-slot bytes start at zero for this path; one read only.
// MemoryStream's explicit invalid/uninitialized-range checks still apply.
std::uint32_t stream_read_u32_00be4300(MemoryStream& stream);
std::uint16_t stream_read_word_00be4340(MemoryStream& stream);
std::uint16_t stream_read_word_00be4320(MemoryStream& stream);
// Native FLD float -> ST0, then font caller stores float. Typed interface
// includes that store; exceptional x87 control/status behavior is unverified.
float stream_read_float_00be4360(MemoryStream& stream);
}
