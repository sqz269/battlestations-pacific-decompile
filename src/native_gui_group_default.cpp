#include "bsp/native_gui_group_default.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
namespace bsp {
void* construct_native_gui_group_default_00ac6f50(void* group,
    const volatile std::uint32_t& one) {
    construct_native_gui_widget_base_00aa9390(group,2,one);
    *static_cast<volatile std::uint32_t*>(group)=0x00d5cb80;
    return group;
}
} // namespace bsp
