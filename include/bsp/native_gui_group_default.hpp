#pragma once
#include <cstdint>
namespace bsp {
// Complete AC6F50[20] normal body: actual raw AA9390(type2), D5CB80 stamp,
// return same actual EC-byte destination. Hidden pool ID+EC stays untouched.
// Native ECX group/EAX same/RET; explicit borrowed current-one C++ ABI.
// No allocation, copy, factory, payload teardown or own EH is supplied.
void* construct_native_gui_group_default_00ac6f50(void* actual_group,
    const volatile std::uint32_t& actual_one_00d7a24c);
} // namespace bsp
