#pragma once

// cc13 pool04 (segment 33): the name() bodies (vtable slot +4h) of nine entity
// order command classes. entity_orders.cpp models the same names as data in
// kEntityOrderCommandClasses; these are the per-address bodies. Every
// descriptive name here is a hypothesis, not a recovered symbol.

namespace bsp {

const char* entity_order_command_name_dropkamikaze_006f8470() noexcept;
const char* entity_order_command_name_depthcharge_006f8510() noexcept;
const char* entity_order_command_name_rocket_006f85b0() noexcept;
const char* entity_order_command_name_strafe_006f8650() noexcept;
const char* entity_order_command_name_kamikaze_006f86f0() noexcept;
const char* entity_order_command_name_dogfight_006f8790() noexcept;
const char* entity_order_command_name_follow_006f8830() noexcept;
const char* entity_order_command_name_moveto_006f8920() noexcept;
const char* entity_order_command_name_tutorial_006f8ad0() noexcept;

}  // namespace bsp
