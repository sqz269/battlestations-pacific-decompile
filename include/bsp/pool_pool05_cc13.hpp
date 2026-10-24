#pragma once
// Packet cc13-pool05 (segment 33): nine command-class virtual slot bodies that
// had only metadata rows (the kEntityOrderCommandClasses / kCommandClassSlot10
// tables in bsp/entity_orders.hpp and bsp/gameplay_seg33_cc13.hpp carry their
// values as data). Each original body is one MOV/XOR to EAX and a RET.
//
// ABI of every slot: __thiscall(ECX = command class, unused), RET, EAX = result.
// The functions here take no object because the bodies never read ECX. The
// name slots return a pointer to the image's literal; these return a C++
// literal with the same bytes, so the pointer value (the image address, given
// per function) differs. Build-tested only, not wired into a game host, not
// ABI-compatible replacements. Names are hypotheses.

#include <cstdint>

namespace bsp::pool_pool05_cc13 {

// 006F8B90: attackmove (vtable 00CFB590) slot +10h -> 0.
std::int32_t attackmove_slot10_006f8b90() noexcept;
// 006F8C00 / 006F8C20: stop (vtable 00CFB5A4) slot +4h name / slot +0Ch category.
const char* stop_name_006f8c00() noexcept;
std::int32_t stop_category_006f8c20() noexcept;
// 006F8C90 / 006F8CB0: retreat (vtable 00CFB5C0) name / category.
const char* retreat_name_006f8c90() noexcept;
std::int32_t retreat_category_006f8cb0() noexcept;
// 006F8D20 / 006F8D40: returntobase (vtable 00CFB5DC) name / category.
const char* returntobase_name_006f8d20() noexcept;
std::int32_t returntobase_category_006f8d40() noexcept;
// 006F8DB0: land (vtable 00CFB600) slot +4h name.
const char* land_name_006f8db0() noexcept;
// 006F8E50: closetoship (vtable 00CFB61C) slot +4h name.
const char* closetoship_name_006f8e50() noexcept;

}  // namespace bsp::pool_pool05_cc13
