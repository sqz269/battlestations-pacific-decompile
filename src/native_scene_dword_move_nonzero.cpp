#include "bsp/native_scene_dword_move_nonzero.hpp"

#include <cstdint>
#include <cstring>

namespace bsp {

void* __stdcall move_native_scene_dword_range_00468f60(
    const void* first, const void* last, void* destination) {
    using Word = std::uint32_t;
    static_assert(sizeof(void*) == sizeof(Word));
    const Word last_address = reinterpret_cast<Word>(last);
    const Word first_address = reinterpret_cast<Word>(first);
    const Word destination_address = reinterpret_cast<Word>(destination);
    const Word difference = last_address - first_address;
    // Exact SAR2 result bits without signed arithmetic or signed-shift assumptions.
    const Word words = (difference >> 2u)
        | ((difference & 0x80000000u) ? 0xc0000000u : 0u);
    const Word bytes = words * 4u;
    const Word result = destination_address + bytes;
    if (words != 0u)
        (void)::memmove_s(destination, bytes, first, bytes); // 00468F82
    return reinterpret_cast<void*>(result);
}

} // namespace bsp
