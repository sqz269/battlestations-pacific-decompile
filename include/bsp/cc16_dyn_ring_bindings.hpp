#pragma once

#include "bsp/cc16_dyn_ring_owner.hpp"

namespace bsp::cc16_dyn_ring_bindings {

// Source composition of the recovered ring owner family. The caller supplies
// the actual native-layout destination/input owner and its actual heap custody.
// No geometry, allocation service, FP mode or result is synthesized. The same
// CRT reference reaches the parent, initializer, preparation and vector copy.
// Ambient math retains native x87 stack-capacity requirements (up to seven
// free entries), controls, store epochs and exception behavior.
//
// This ordinary C++ entry is not the original two-stack-argument RET8 ABI.
// It establishes no original caller reachability, game/root activation or
// runtime validation. Named dependency acceptance belongs to the integrator.
cc16_dyn_ring_owner::DynRingOwner* construct_dyn_ring_owner_bound_00c4d5a0(
    cc16_dyn_ring_owner::DynRingOwner& destination,
    const cc16_dyn_ring_init::DynRingOwnerCount& input_owner,
    const cc15_dyn_live::DynLiveCrt& crt);

}  // namespace bsp::cc16_dyn_ring_bindings
