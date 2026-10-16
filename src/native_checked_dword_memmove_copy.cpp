#include "bsp/native_checked_dword_memmove_copy.hpp"

#include <cstdint>
#include <cstring>

namespace bsp {

void* __stdcall copy_native_checked_dword_memmove_00440c70(
    const void* first, const void* last, void* destination) {
    using Word = std::uint32_t;
    static_assert(sizeof(void*) == sizeof(Word));
    const Word last_address = reinterpret_cast<Word>(last);
    const Word first_address = reinterpret_cast<Word>(first);
    const Word destination_address = reinterpret_cast<Word>(destination);
    const Word difference = last_address - first_address;
    // Exact SAR2 bits; unsigned arithmetic avoids signed overflow and pointerdiff.
    const Word words = (difference >> 2U)
        | ((difference & 0x80000000U) ? 0xc0000000U : 0U);
    const Word bytes = words * 4U;
    const Word result = destination_address + bytes;
    if (words != 0U) {
        (void)::memmove_s(destination, bytes, first, bytes);
    }
    return reinterpret_cast<void*>(result);
}

} // namespace bsp
