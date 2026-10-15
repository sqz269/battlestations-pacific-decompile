#include "bsp/native_scene_dword_move.hpp"

#include <cstdint>
#include <cstring>

namespace bsp {

void* __cdecl move_native_scene_dword_range_00467d90(
    const void* first, const void* last, void* destination) {
    using U = std::uint32_t;
    static_assert(sizeof(void*) == sizeof(U));
    const U last_address = reinterpret_cast<U>(last);
    const U first_address = reinterpret_cast<U>(first);
    const U difference = last_address - first_address;
    // Exact SAR2 bits without signed conversion or a signed-shift assumption.
    const U words = (difference >> 2u)
        | ((difference & 0x80000000u) ? 0xc0000000u : 0u);
    const U destination_address = reinterpret_cast<U>(destination);
    const U bytes = words * 4u;
    const U result = bytes + destination_address;
    if (words != 0 && (words & 0x80000000u) == 0)
        (void)::memmove_s(destination, bytes, first, bytes);
    return reinterpret_cast<void*>(result);
}

} // namespace bsp
