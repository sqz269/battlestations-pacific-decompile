#pragma once

#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native World current-storage tick requires MSVC Win32 assembly.
#endif

namespace bsp {

// Borrow the same actual live clock used by its qualified reset/step writers,
// genuine current operands, and the intended returning-capable validator.
// The context supplies no owner, initialization, table, allocator or binding.
struct NativeWorldCurrentTickContext {
    const volatile float& mission_clock_00f876a4;
    const volatile float& negative_zero_00d7a208;
    const volatile float& one_00d7a24c;
    const SingletonLifetimeCallbacks& validation;
};

// Complete 00904BF0 entity walk and unconditional concrete 00904600 tail.
// Native ABI: ECX World, one float stack argument, RET4. This explicit-context
// C++ interface is new. World+4 points to an actual first/last/count header;
// current entities expose active+5C, next+38 and callable primary slot+DC
// (__thiscall receiver,float; RET4). Each entity survives its post-call+38 read.
void update_native_world_entities_00904bf0(
    void* actual_world, float delta, const NativeWorldCurrentTickContext&);

// Complete 00904600. Native ABI: ECX World, unused float stack argument, RET4.
// World+4B0 is the retained checked-list header; its current sentinel/count are
// at +4/+8. Actual 6Ch nodes have next/previous at +0/+4 and a 64h record at+8.
// The caller supplies coherent live storage and actual entity tables: slot+88
// takes one callee-popped matrix pointer, slot+D8 takes no argument, both with
// ECX receiver. Their results are unused. Children support the raw 0042ED50
// contract. Expired nodes belong to singleton_lifetime_allocate/free's domain.
void run_native_world_matrix_pass_00904600(
    void* actual_world, float unused_delta, const NativeWorldCurrentTickContext&);

// All borrows, callable methods and later-read storage survive every reached
// callback. validation.invalid_parameter must name the intended live service;
// it may return and mutate observed fields. Neither entry invents null/cycle
// repair, a removal cap, rollback or an exception policy. Completed writes
// remain when a Source call throws. No World-vtable read, producer, application
// binding, synchronization, general native fault/SEH/private-frame equivalence
// or gameplay result is supplied. Descriptive names are hypotheses.

} // namespace bsp
