#pragma once

#include "bsp/cc15_dyn_live.hpp"

namespace bsp::cc16_dyn_sap_retire {

// 00C4C550..00C4C5C6: EBX manager, stack endpoint, RET4. Retire the
// endpoint's referenced pair records, erasing the other endpoint's reference
// and recycling each pair. Outer count is live; own references are not cleared.
void retire_endpoint_pairs_00c4c550(
    void* manager, void* endpoint, const cc15_dyn_live::DynLiveCrt& crt);

// RTTI 00D7A13C slot1, 00C4CC90..00C4CD9C: ECX manager, stack endpoint,
// RET4. Active objects remove all three axis endpoints then retire pairs;
// pending objects compact the pending-pointer array. Free the actual reference
// buffer, unlink the object and recycle it into the pool selected before free.
void remove_and_recycle_endpoint_00c4cc90(
    void* manager, void* endpoint, const cc15_dyn_live::DynLiveCrt& crt);

// Actual raw storage and required native CRT custody only. No fabricated
// class/vtable, missing-item guard, default provider or exception cleanup.
// New void C++ interfaces assert no consumed semantic EAX result, original
// register ABI, fault-state equivalence or game activation.
}  // namespace bsp::cc16_dyn_sap_retire
