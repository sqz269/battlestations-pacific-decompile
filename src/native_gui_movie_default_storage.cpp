#include "bsp/native_gui_movie_default_storage.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI Movie default construction requires MSVC Win32.
#endif
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

namespace bsp {
void* construct_native_gui_movie_default_00aafe20(void* actual_storage,
    const volatile std::uint32_t& actual_one_00d7a24c) {
    auto* const bytes = static_cast<std::byte*>(actual_storage);
    construct_native_gui_widget_base_00aa9390(actual_storage, 10, actual_one_00d7a24c);
    *reinterpret_cast<volatile std::uint32_t*>(bytes) = 0x00d5c270u; // AAFE42
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0xec) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0xf0) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0xf8) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0xfc) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x100) = 0;
    *reinterpret_cast<volatile std::uint8_t*>(bytes + 0x104) = 0;
    *reinterpret_cast<volatile std::uint8_t*>(bytes + 0x105) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x108) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x110) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x114) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x118) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x124) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x128) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x12c) = 0;
    return actual_storage;
}
} // namespace bsp
