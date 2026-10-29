#pragma once

#include "bsp/cc15_dyn_live.hpp"
#include "bsp/dyn_profile_scopes.hpp"

namespace bsp::cc16_dyn_radix_manager_update {

// Borrow the actual image names; names are not replaced with host literals.
// The profile cache uses IDs 9..13 independently of these name pointers.
struct RadixManagerProfileNames {
    const char* sap_sort_00d79c80;
    const char* sap_sweep_00d79c88;
    const char* sap_object_num_00d79c94;
    const char* sap_possible_pair_num_00d79ca4;
    const char* sap_pair_num_00d79cb8;
};

// Complete normal instruction path of 00C5A8B0..00C5B175, native ECX=raw
// manager, no stack arguments/plain RET. The unrelated 00C5B180 wrapper is out.
// This is a new C++ interface, not an original ABI/vtable adapter or activation.
// All arguments are required genuine caller-owned storage/providers, retained
// through each call and exception cleanup. The manager has the actual native
// endpoint/scratch/pair vectors and selected-axis word; no class is inferred.
// Profile context borrows the actual 0109E9F8 publication cell, real 9Ch profile,
// real 48h nodes and their allocation domain. CRT custody remains the caller's.
// No checks, default globals, replacement names or half constants are supplied.
//
// Ambient x87/MXCSR state is inherited. No control-word change, FP conversion
// in a wrapper, algebraic replacement or heap histogram is introduced.
// Compiler-EH bridges run the four evidenced real profile cleanup actions for
// C++ exceptions from bridged calls. Sort has no unwind action. Original FH3/
// SEH transport, faults, timing/frame identity, secondary cleanup exceptions
// and binary ABI are not supplied. No game,
// differential fixture or live binding proof is claimed. See the packet doc.
void update_radix_manager_00c5a8b0(
    void* manager,
    const cc15_dyn_live::DynLiveCrt& crt,
    const DynProfileScopeContext& profile,
    const volatile std::uint32_t& actual_feature_word_0109eea4,
    const volatile double& actual_half_00d7a280,
    const RadixManagerProfileNames& actual_names);

// Additive qualified cold-profile append. Same actual profile memory plus its
// required matching flat BF65AC; null is rejected before manager/profile access
// or mutation. The old six-argument entry remains normal-only. The private
// native-context prefix, instruction kernel and four scope-leave actions stay
// unchanged; real five-argument C50390 handles its own partial construction.
// Original naked-frame rethrow/FH3/SEH/fault transport remains unproven.
// This does not bind a radix owner/vtable, propagate other callers or activate.
void update_radix_manager_00c5a8b0(
    void* manager,
    const cc15_dyn_live::DynLiveCrt& crt,
    const DynProfileScopeContext& profile,
    const volatile std::uint32_t& actual_feature_word_0109eea4,
    const volatile double& actual_half_00d7a280,
    const RadixManagerProfileNames& actual_names,
    DynProfilePartialNodeDelete delete_partial_00bf65ac);

} // namespace bsp::cc16_dyn_radix_manager_update
