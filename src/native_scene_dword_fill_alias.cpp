#include "bsp/native_scene_dword_fill_alias.hpp"
#include "bsp/native_dword_fill_ranges.hpp"

namespace bsp {

void* __cdecl fill_native_dword_range_00468440(void* first,
    const void* last, const void* value_address) {
    return fill_native_dword_range_0043f9e0(first, last, value_address);
}

} // namespace bsp
