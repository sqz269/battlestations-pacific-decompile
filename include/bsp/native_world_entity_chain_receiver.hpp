#pragma once

#if defined(_MSC_VER) && defined(_M_IX86)

namespace bsp {

// Complete 00904390: ECX actual World, fresh pointer at +4, then tail transfer
// to the qualified actual 009041A0 chain retirement. No input EDX or explicit
// stack argument is consumed. Receiver, genuine header and current callable
// entity lifetimes/progress remain caller contracts. No World/table ownership,
// Native fault/EH/runtime or production/gameplay compatibility is established.
void __fastcall retire_native_world_entities_00904390(void* actual_world);

} // namespace bsp

#endif
