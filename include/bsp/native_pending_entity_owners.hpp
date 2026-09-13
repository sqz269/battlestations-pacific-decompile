#pragma once

#include "bsp/native_live_effect_manager.hpp"

namespace bsp {

// Exact shared raw 0Ch layout, established from the node producers. Values are
// borrowed entity pointers. These aliases introduce no container or ownership.
using NativePendingEntityNode = NativeEffectDeletionNode;
using NativePendingEntityListStorage = NativeEffectDeletionListStorage;

// Actual adjacent owners F899A8/F899B4. No implicit initialization/destruction.
struct NativePendingEntityOwners {
    NativePendingEntityListStorage destroy_00f899a8;
    NativePendingEntityListStorage kill_00f899b4;
};
static_assert(sizeof(NativePendingEntityOwners) == 0x18);
static_assert(offsetof(NativePendingEntityOwners, kill_00f899b4) == 0x0c);

// Required actual CRT domain binding. It must retain and eventually dispatch
// the supplied native shutdown identity against these SAME owner bytes, with
// CRT registration order/failure semantics. Context and owners must survive
// registration through dispatch. No default callback or private exit registry.
struct NativePendingEntityCrtRegistration {
    void* context;
    int (*register_atexit)(void*, std::uint32_t native_shutdown) noexcept;
};

// Complete CD3910[37]/CD3940[37]: no native inputs, EAX _atexit status, RET.
// Allocate/self-link 0Ch sentinel, publish head, zero count, register CDF4A0/B0.
// Preserve owner+00 and sentinel payload+08. Registration failure retains the
// initialized list and its actual return status; allocation failure publishes
// nothing and does not register. A null registration callback is an unbound
// source interface (rejected before touching storage), not a native code path.
int initialize_native_pending_destroy_owner_00cd3910(
    NativePendingEntityOwners&, const NativePendingEntityCrtRegistration&);
int initialize_native_pending_kill_owner_00cd3940(
    NativePendingEntityOwners&, const NativePendingEntityCrtRegistration&);

// Complete CDF4A0[10]/CDF4B0[10], no native inputs, tail JMP 924A70.
// Detach ring and zero count; free nodes in next-link order, then sentinel;
// clear head. Never delete payloads. Require an initialized intact finite ring.
// Preserve owner+00 and the other list. No locking or event delivery occurs.
void destroy_native_pending_destroy_owner_00cdf4a0(NativePendingEntityOwners&) noexcept;
void destroy_native_pending_kill_owner_00cdf4b0(NativePendingEntityOwners&) noexcept;

// New C++ ABI. Source reuse of 4C3200/4C5940 is proved by complete PE spans
// with relative CALL operands normalized to their actual targets. Ghidra's
// stored 924A70/4C5940 bodies still omit their final continuations; full bytes
// and limitations are retained in reports/native_pending_entity_owners.json.
// Producer/cancel/drain behavior and native exception dispatch are not provided
// by this owner component. See docs/NATIVE_PENDING_ENTITY_OWNERS.md.
} // namespace bsp
