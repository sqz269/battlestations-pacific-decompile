#pragma once

#include "bsp/cc15_dyn_live.hpp"

namespace bsp::cc16_dyn_radix_retire {

using ObjectFree00BF65AC = void (*)(void*);

// 00C40CC0..00C40D48: EAX object, ESI manager, RET0. Replace its packed
// two-DWORD slot with the last entry and publish that moved object's WORD
// index at +20h. Preserve the original uint32 underflow-only growth arm.
void remove_radix_slot_00c40cc0(void* manager, void* object,
                              const cc15_dyn_live::DynLiveCrt& crt);

// Existing RTTI identifies 00D7A118 slot1 as 00C40D50..00C40D6C.
// Native ECX manager, stack object, RET4; remove slot then free the same
// captured object through the distinct original 00BF65AC binding.
void retire_radix_object_00c40d50(void* manager, void* object,
                                const cc15_dyn_live::DynLiveCrt& crt,
                                ObjectFree00BF65AC free_object);

// Real object/manager/heap custody and the BF65AC binding are required.
// No default allocator/free, malformed-state guard, exception rollback,
// vtable fabrication, original register ABI or runtime activation is added.
}  // namespace bsp::cc16_dyn_radix_retire
