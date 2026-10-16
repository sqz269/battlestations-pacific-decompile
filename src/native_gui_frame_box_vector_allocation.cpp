#include "bsp/native_gui_frame_box_vector_allocation.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <new>

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(std::size_t) == 4);

void* __fastcall allocate_native_gui_frame_box_records_00acef10(
    std::uint32_t count, void*) {
    // Native positive-count DIV admits FFFFFFFF/count >=2Ch, including the
    // largest product FFFFFFFCh. Overflow uses the established SDK boundary.
    if (count > 0x05d1745du)
        throw std::bad_alloc();
    const std::uint32_t bytes = count * 0x2cu;
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

} // namespace bsp
