// cc13 pool03 (segment 33): command-class vtable leaves. Descriptive names are hypotheses.
//
// Evidence common to every body below (cc13-poolw3, 2026-10-24):
// - Each body is the whole function: `ghidra bytes` shows the RET followed by CC padding up to
//   the next 10h-aligned function, and `ghidra xrefs` shows exactly one DATA reference, the
//   vtable slot named on each body. No callers, no callees.
// - Original ABI: virtual __thiscall; ECX (`this`) is never read, no stack arguments, plain RET.
//   No x87, no globals read, no EH frame (EH states: none), no register inputs besides ECX.
// - The vtable bases are the kEntityOrderCommandClasses rows (src/entity_orders.cpp):
//   settarget 00CFB3A4, cleartarget 00CFB3B8, clearorders 00CFB3CC, artillery 00CFB3E0,
//   torpedo 00CFB400, divebomb 00CFB41C, levelbomb 00CFB43C.
// Uncertainty: the role of slot +10h is inferred from its one consumer, the menu comparator
// 006F7B30 (bsp/gameplay_seg33_cc13.hpp); the C++ returns the literal text, not the .rdata
// address.

#include "bsp/pool_pool03_cc13.hpp"

namespace bsp {

// 006F80B0, slot +4h of 00CFB3B8 (DATA xref 00CFB3BC). B8 00 8B CF 00 / C3:
// MOV EAX,0x00CF8B00 ("cleartarget\0", bytes 63 6C 65 61 72 74 61 72 67 65 74 00).
const char* command_class_cleartarget_name_006f80b0() noexcept { return "cleartarget"; }

// 006F8150, slot +4h of 00CFB3CC (DATA xref 00CFB3D0). MOV EAX,0x00CF8AF4 / RET;
// 00CF8AF4 = "clearorders\0".
const char* command_class_clearorders_name_006f8150() noexcept { return "clearorders"; }

// 006F81F0, slot +4h of 00CFB3E0 (DATA xref 00CFB3E4). MOV EAX,0x00CFB3F4 / RET;
// 00CFB3F4 = "artillery\0".
const char* command_class_artillery_name_006f81f0() noexcept { return "artillery"; }

// 006F8290, slot +4h of 00CFB400 (DATA xref 00CFB404). MOV EAX,0x00CFB414 / RET;
// 00CFB414 = "torpedo\0".
const char* command_class_torpedo_name_006f8290() noexcept { return "torpedo"; }

// 006F8330, slot +4h of 00CFB41C (DATA xref 00CFB420). MOV EAX,0x00CFB430 / RET;
// 00CFB430 = "divebomb\0".
const char* command_class_divebomb_name_006f8330() noexcept { return "divebomb"; }

// 006F83D0, slot +4h of 00CFB43C (DATA xref 00CFB440). MOV EAX,0x00CFB450 / RET;
// 00CFB450 = "levelbomb\0".
const char* command_class_levelbomb_name_006f83d0() noexcept { return "levelbomb"; }

// 006F8030, slot +10h of 00CFB3A4 (DATA xref 00CFB3B4). 33 C0 / C3: XOR EAX,EAX / RET.
int command_class_settarget_slot10_006f8030() noexcept { return 0; }

// 006F80D0, slot +10h of 00CFB3B8 (DATA xref 00CFB3C8). XOR EAX,EAX / RET.
int command_class_cleartarget_slot10_006f80d0() noexcept { return 0; }

// 006F8170, slot +10h of 00CFB3CC (DATA xref 00CFB3DC). XOR EAX,EAX / RET.
int command_class_clearorders_slot10_006f8170() noexcept { return 0; }

}  // namespace bsp
