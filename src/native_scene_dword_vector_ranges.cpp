#include "bsp/native_scene_dword_vector_ranges.hpp"

#include "bsp/native_dword_fill_ranges.hpp"

#include <cstring>

namespace bsp {

void* __cdecl copy_backward_native_scene_dword_range_00468460(
    const void* first, const void* last, void* destination_end) {
    using U = std::uint32_t;
    static_assert(sizeof(void*) == sizeof(U));
    const U last_address = reinterpret_cast<U>(last);
    const U first_address = reinterpret_cast<U>(first);
    const U difference = last_address - first_address;
    const U words = (difference >> 2u)
        | ((difference & 0x80000000u) ? 0xc0000000u : 0u);
    const U end = reinterpret_cast<U>(destination_end);
    const U bytes = words * 4u;
    const U start = end - bytes;
    if (words != 0u && (words & 0x80000000u) == 0u)
        (void)::memmove_s(reinterpret_cast<void*>(start), bytes,
            reinterpret_cast<const void*>(first_address), bytes);
    return reinterpret_cast<void*>(start);
}

void* __stdcall fill_native_scene_dword_count_00469950(
    void* first, std::uint32_t count, const void* value_address) {
    if (count == 0u)
        return first;
    return fill_native_dword_count_00441180(first, count, value_address);
}

} // namespace bsp
