#include "bsp/native_gui_icon_state_allocation_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Source allocation supplier requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(std::size_t) == 4);

void* allocate_native_gui_icon_state_backing_00ab1520(std::uint32_t count_ecx) {
    const std::uint32_t count = count_ecx;
    if (count != 0 && 0xffffffffu / count < 0x40u)
        throw std::bad_alloc(); // Qualified Source SDK error, not Native EH layout.
    const std::uint32_t bytes = count << 6;
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
} // namespace bsp
