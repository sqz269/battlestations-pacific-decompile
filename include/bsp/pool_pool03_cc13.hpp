#pragma once

// cc13 pool03 (segment 33): command-class vtable leaves for settarget .. levelbomb.
// Slot +4h answers the class's name literal, slot +10h the menu sort key (kCommandClassSlot10 in
// bsp/gameplay_seg33_cc13.hpp). The class table itself is bsp::kEntityOrderCommandClasses
// (bsp/entity_orders.hpp). Build only: no host path calls these.

namespace bsp {

// Slot +4h, __thiscall(this) with `this` unused, no stack args, RET: MOV EAX,imm32 / RET.
// Each returns a pointer to a NUL-terminated .rdata literal; the C++ returns the same text.
const char* command_class_cleartarget_name_006f80b0() noexcept;   // 00CF8B00 "cleartarget"
const char* command_class_clearorders_name_006f8150() noexcept;   // 00CF8AF4 "clearorders"
const char* command_class_artillery_name_006f81f0() noexcept;     // 00CFB3F4 "artillery"
const char* command_class_torpedo_name_006f8290() noexcept;       // 00CFB414 "torpedo"
const char* command_class_divebomb_name_006f8330() noexcept;      // 00CFB430 "divebomb"
const char* command_class_levelbomb_name_006f83d0() noexcept;     // 00CFB450 "levelbomb"

// Slot +10h, __thiscall(this) with `this` unused, RET: XOR EAX,EAX / RET (menu sort key 0).
int command_class_settarget_slot10_006f8030() noexcept;
int command_class_cleartarget_slot10_006f80d0() noexcept;
int command_class_clearorders_slot10_006f8170() noexcept;

}  // namespace bsp
