#include "bsp/native_font_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdint>

namespace bsp {
void* allocate_native_font_tree_head_00ad42c0() {
    static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4,
        "AD42C0 raw storage requires Win32 pointer widths");
    static_assert(sizeof(std::uint32_t) == 4, "AD42C0 DWORD width");
    void* const result = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x18u, 0x18u});
    const auto base = reinterpret_cast<std::uintptr_t>(result);
    if (base != 0)
        *reinterpret_cast<volatile std::uint32_t*>(base) = 0;
    const auto parent = base + std::uintptr_t{4};
    if (parent != 0)
        *reinterpret_cast<volatile std::uint32_t*>(parent) = 0;
    const auto right = base + std::uintptr_t{8};
    if (right != 0)
        *reinterpret_cast<volatile std::uint32_t*>(right) = 0;
    *reinterpret_cast<volatile std::uint8_t*>(base + 0x14u) = 1;
    *reinterpret_cast<volatile std::uint8_t*>(base + 0x15u) = 0;
    return result;
}
} // namespace bsp
