// Packet cc13-pool05: see include/bsp/pool_pool05_cc13.hpp. Build-tested only.
// Evidence for every body: the complete listing (two instructions), the vtable
// dwords read from the image (slot +4h / +0Ch / +10h of each vtable point at
// these addresses) and the literal bytes at the returned address. No EH frame,
// no x87, no globals read; ECX is never used.
#include "bsp/pool_pool05_cc13.hpp"

namespace bsp::pool_pool05_cc13 {

// 006F8B90: XOR EAX,EAX; RET. [00CFB5A0] (attackmove vtable 00CFB590 + 10h).
// Slot +10h is the menu sort key compared by 006F7B30; its meaning is unknown.
std::int32_t attackmove_slot10_006f8b90() noexcept { return 0; }

// 006F8C00: MOV EAX,00CFB5B8; RET. [00CFB5A8] (stop vtable 00CFB5A4 + 4h).
// 00CFB5B8 = "stop".
const char* stop_name_006f8c00() noexcept { return "stop"; }

// 006F8C20: MOV EAX,3; RET. [00CFB5B0] (stop vtable + 0Ch).
std::int32_t stop_category_006f8c20() noexcept { return 3; }

// 006F8C90: MOV EAX,00CFB5D4; RET. [00CFB5C4] (retreat vtable 00CFB5C0 + 4h).
// 00CFB5D4 = "retreat".
const char* retreat_name_006f8c90() noexcept { return "retreat"; }

// 006F8CB0: MOV EAX,3; RET. [00CFB5CC] (retreat vtable + 0Ch).
std::int32_t retreat_category_006f8cb0() noexcept { return 3; }

// 006F8D20: MOV EAX,00CFB5F0; RET. [00CFB5E0] (returntobase vtable 00CFB5DC + 4h).
// 00CFB5F0 = "returntobase".
const char* returntobase_name_006f8d20() noexcept { return "returntobase"; }

// 006F8D40: MOV EAX,3; RET. [00CFB5E8] (returntobase vtable + 0Ch).
std::int32_t returntobase_category_006f8d40() noexcept { return 3; }

// 006F8DB0: MOV EAX,00CFB614; RET. [00CFB604] (land vtable 00CFB600 + 4h).
// 00CFB614 = "land".
const char* land_name_006f8db0() noexcept { return "land"; }

// 006F8E50: MOV EAX,00CFB630; RET. [00CFB620] (closetoship vtable 00CFB61C + 4h).
// 00CFB630 = "closetoship".
const char* closetoship_name_006f8e50() noexcept { return "closetoship"; }

}  // namespace bsp::pool_pool05_cc13
