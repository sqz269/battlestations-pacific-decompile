#pragma once

#include <cstdint>

namespace bsp::cc16_dyn_triplet {

// 00C39040..00C390F9: EAX left, EDX right, no stack arguments, RET0;
// full EAX is zero/one. EBX/ESI/EDI are saved; EBP is untouched.
// Compare three raw DWORDs up to cyclic rotation, preserving phase/load order.
// No null/length guard or input write is introduced. These explicit C++
// parameters are not an adapter for the original register ABI or fault state.
std::uint32_t cyclic_equal_00c39040(
    const std::uint32_t* left, const std::uint32_t* right) noexcept;

// 00C48140..00C48167: EAX left, one stack right argument, RET4.
// The wrapper captures left words in order 1,2,0, arranges [2,1,0], then
// invokes 00C39040 with that local triplet and the unchanged right pointer.
std::uint32_t reversed_cyclic_equal_00c48140(
    const std::uint32_t* left, const std::uint32_t* right) noexcept;

}  // namespace bsp::cc16_dyn_triplet
