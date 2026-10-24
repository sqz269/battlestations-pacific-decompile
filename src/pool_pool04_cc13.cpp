#include "bsp/pool_pool04_cc13.hpp"

namespace bsp {

// Nine name() bodies, vtable slot +4h of the entity order command classes that
// entity_orders.cpp lists in kEntityOrderCommandClasses (constructors
// 00CCE500 + 40h*k). Descriptive names are hypotheses.
//
// Evidence: each body is its whole listing, `MOV EAX,<literal>; RET` (two
// instructions, 6 bytes). The slot dword and the literal bytes were read from
// the image (ghidra bytes, 2026-10-24):
//   address   vtable   slot +4h  literal   bytes
//   006F8470  00CFB45C 00CFB460  00CFB470  "dropkamikaze"
//   006F8510  00CFB480 00CFB484  00CFB494  "depthcharge"
//   006F85B0  00CFB4A0 00CFB4A4  00CFB4B4  "rocket"
//   006F8650  00CFB4BC 00CFB4C0  00CFB4D0  "strafe"
//   006F86F0  00CFB4D8 00CFB4DC  00CFB4EC  "kamikaze"
//   006F8790  00CFB4F8 00CFB4FC  00CFB50C  "dogfight"
//   006F8830  00CFB518 00CFB51C  00CFB52C  "follow"
//   006F8920  00CFB534 00CFB538  00CFB54C  "moveto"
//   006F8AD0  00CFB570 00CFB574  00CFB584  "tutorial"
// Each address has one data reference, its vtable slot, and no direct callers.
//
// Original ABI: __thiscall virtual, ECX = this (unused), no stack arguments,
// plain RET; returns the address of a static literal in EAX. No EH state, no
// x87, no globals read. These bodies take no `this`, since the original never
// reads ECX; the literal is the same text, not the same address.
// Uncertainty: none in the bodies. Whether the base slot (006F7F30, "undefined
// command name") is ever reached for these classes is outside this packet.

const char* entity_order_command_name_dropkamikaze_006f8470() noexcept { return "dropkamikaze"; }
const char* entity_order_command_name_depthcharge_006f8510() noexcept { return "depthcharge"; }
const char* entity_order_command_name_rocket_006f85b0() noexcept { return "rocket"; }
const char* entity_order_command_name_strafe_006f8650() noexcept { return "strafe"; }
const char* entity_order_command_name_kamikaze_006f86f0() noexcept { return "kamikaze"; }
const char* entity_order_command_name_dogfight_006f8790() noexcept { return "dogfight"; }
const char* entity_order_command_name_follow_006f8830() noexcept { return "follow"; }
const char* entity_order_command_name_moveto_006f8920() noexcept { return "moveto"; }
const char* entity_order_command_name_tutorial_006f8ad0() noexcept { return "tutorial"; }

}  // namespace bsp
