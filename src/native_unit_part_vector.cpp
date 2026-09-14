#include "bsp/native_unit_part_vector.hpp"
#include "bsp/native_input_settings_vector_storage.hpp"

namespace bsp {
void resize_native_unit_part_pointers_0087b460(void* header,
    std::uint32_t count, std::uint32_t fill) {
    resize_native_checked_dword_storage(header,count,fill,
        NativeCheckedDwordPublication::capacity_end_begin);
}
} // namespace bsp
