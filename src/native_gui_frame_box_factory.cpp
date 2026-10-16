#include "bsp/native_gui_frame_box_factory.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI FrameBox construction requires MSVC Win32.
#endif
static_assert(sizeof(void*) == 4);

namespace bsp {
void* construct_native_gui_frame_box_default_00ad2600(
    void* destination, NativeGuiFrameBoxDefaultBindings& bindings) {
    auto* const bytes = static_cast<std::byte*>(destination);
    construct_native_gui_widget_base_00aa9390(
        destination, 18, bindings.actual_one_00d7a24c);
    const std::uint32_t x = bindings.actual_frame_x_00d5d11c;
    *reinterpret_cast<volatile std::uint32_t*>(bytes) = 0x00d5d130u;
    *reinterpret_cast<volatile std::uint16_t*>(bytes + 0xec) = 0xffffu;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0xf4) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0xf8) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0xfc) = 0;
    *reinterpret_cast<volatile std::uint8_t*>(bytes + 0x100) = 1;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x104) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x108) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x10c) = x;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x110) = x;
    const std::uint32_t y = bindings.actual_frame_y_00d5d10c;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x114) = y;
    *reinterpret_cast<volatile std::uint32_t*>(bytes + 0x118) = y;
    return destination;
}
} // namespace bsp

namespace bsp {
std::uint16_t __fastcall native_gui_frame_box_current_state_bits_00ad26d0(
    const void* actual_frame_box) noexcept {
    return *reinterpret_cast<const volatile std::uint16_t*>(
        static_cast<const std::byte*>(actual_frame_box) + 0xec);
}
} // namespace bsp
