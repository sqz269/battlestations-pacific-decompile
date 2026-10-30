#pragma once

#include "bsp/avoid_zone_dyn_hull.hpp"

namespace bsp {
struct alignas(4) DynProfileNodeStorage { unsigned char bytes[0x48]; };
struct alignas(4) DynProfileStorage { unsigned char bytes[0x9c]; };
struct alignas(4) DynEngineStorage { unsigned char bytes[0x14]; };

// Borrow the same publication cells used by world/scene construction. Root's
// native string is at 00D79DC8; its bytes are copied through the existing legacy
// string implementation. No replacement task manager or global is introduced.
struct DynEngineRuntimeContext {
    AvoidZoneDynHullMemory memory;
    void** engine_slot_0109e9fc;
    void** profile_slot_0109e9f8;
    const char* root_name_00d79dc8;
};

// 00C44000..00C44081. Native EDX=name, stack(node*, word2C), EAX=node, RET8.
// Complete normal storage construction; reuses NativeLegacySboStringStorage.
// Node+00/+10/+44 and the unused inline-string bytes remain untouched. The
// arbitrary word at+2C is preserved, without inferring a new ownership rule.
DynProfileNodeStorage* dyn_profile_node_construct_00c44000(
    DynProfileNodeStorage&, const char* name, std::uint32_t word_2c);

// Additive cleanup-qualified interface; the existing three-argument overload
// remains normal-only. vector_memory.release is the required genuine BF6989
// provider for this node's child vector and must return normally. C44000 state0
// begins after +4/+8/+C
// zeroing. A C++ exception then composes real0040A900 current-data capture/
// conditional release with no field clear, before rethrowing. No raw-node free
// occurs here; C50390 or qualified C50310 supplies separate BF65AC ownership.
// Original FH3/SEH, fault and frame transport are not supplied by this API.
DynProfileNodeStorage* dyn_profile_node_construct_00c44000(
    DynProfileNodeStorage&, const char* name, std::uint32_t word_2c,
    const AvoidZoneDynHullMemory& vector_memory);

// 00C50310..00C50382. Native ESI=profile, EAX=profile, plain RET.
// Allocates the actual48h Root node, publishes the profile after its three head
// words, then clears the90h tail. It does not initialize the engine global.
// This existing overload remains normal-only.
DynProfileStorage* dyn_profile_construct_00c50310(
    DynProfileStorage&, const DynEngineRuntimeContext&);

// Required genuine matching-heap raw-node release, distinct from the context's
// BF6989 vector release. Must return normally. Kept here without including
// dyn_profile_scopes.hpp (which itself depends on this header).
using DynProfileRootPartialNodeDelete = void (__cdecl*)(void* original_allocation);

// Additive qualified Root interface; rejects a null provider before caller
// data access. Allocate48 BEFORE the catch, then compose qualified C44000's
// real A900 vector cleanup. Root state0 covers that construction, all profile
// head stores, actual publication and the tail source operation through return.
// A C++ exception frees ONLY the originally captured raw allocation and rethrows;
// no head/publication clear, current-node reload, retry or rollback is added.
// The existing std::memset tail is not native BF79F0/fault/SEH proof. Original
// FH3/frame/ABI and C55EA0/C55F50 caller propagation remain separate. The same
// actual memory/publication and matching provider must be supplied and retained;
// a nonnull pointer alone does not authenticate an arbitrary custom context.
DynProfileStorage* dyn_profile_construct_00c50310(
    DynProfileStorage&, const DynEngineRuntimeContext&,
    DynProfileRootPartialNodeDelete delete_partial_00bf65ac);

// 00C55EA0..00C55F4E. Native stack(engine*,descriptor*), EAX=engine, RET8.
// Fresh14h engine owns its profile and real358h task manager. Descriptor points
// to its consumed first dword; it is read only after task-manager allocation.
DynEngineStorage* dyn_engine_construct_00c55ea0(DynEngineStorage&,
    const std::uint32_t* descriptor_worker_count, const DynEngineRuntimeContext&);

// Additive C++ cleanup-qualified C55EA0. Required genuine matching raw BF65AC
// provider covers the SAME scalar-new domain as Root 48h/profile 9Ch/task 358h/
// engine 14h; vector BF6989 remains the actual context.memory.release service.
// Reject a null raw provider before caller data access. Initial engine+0/+4/+8
// zeros precede the source outer current-vector catch. The profile capture is
// guarded only around qualified Root, ending BEFORE engine+C is stored. Task
// allocation follows that store; its captured raw guard covers the single late
// volatile descriptor read, qualified task construction, engine+10 and return.
// A null task allocation skips descriptor access and child construction.
// Raw actions free their original capture unconditionally; the outer action
// captures CURRENT engine vector data once and releases iff nonnull. No field/
// publication clear, child destructor, orphan repair or rollback is added.
// Actual fresh disjoint storage, stable contexts/cells and matching providers
// must outlive all nested work/cleanup; cleanup providers must return normally.
// A nonnull function pointer does not authenticate arbitrary custom custody.
// These are C++ catch extents; emitted /EHsc hoists, native faults/FH3/SEH,
// frame/ABI and secondary-cleanup transport remain separate. Old API is unchanged.
DynEngineStorage* dyn_engine_construct_qualified_00c55ea0(DynEngineStorage&,
    const volatile std::uint32_t* descriptor_worker_count,
    const DynEngineRuntimeContext&, DynProfileRootPartialNodeDelete delete_partial_00bf65ac);

// 00C55F50..00C55FBE. Native ECX=descriptor*, EAX=existing/new engine, RET.
// Existing nonnull publication returns immediately. Otherwise publish only
// after the complete constructor returns. The native allocator throws on
// exhaustion; explicit null-return branches are retained, but native EH cleanup,
// destruction, profile timing and task execution are separate responsibilities.
// These are new C++ interfaces, not native ABI replacements. Inputs and borrowed
// cells must be valid for every path that reads them; no concurrent startup.
DynEngineStorage* dyn_engine_ensure_00c55f50(
    const std::uint32_t* descriptor_worker_count, const DynEngineRuntimeContext&);

// Additive qualified cold ensure. The first actual publication capture returns
// immediately when nonnull, before descriptor access, allocator/provider calls
// or raw-provider validation; it does not requalify that existing engine.
// Cold null-provider rejection precedes 14h allocation. That allocation is
// OUTSIDE acquired raw cleanup. Conditional qualified construction, current
// publication (including null) and return stay inside the captured 14h catch.
// Failure frees ONLY the originally captured 14h allocation, leaving reached
// child/orphan/publication effects. No post-callback recheck or legacy fallback.
// Same actual inputs/custody and C++/fault/ABI limits as the qualified constructor.
// Production/Game callers and whole partial-failure lifetime remain unbound here.
DynEngineStorage* dyn_engine_ensure_qualified_00c55f50(
    const volatile std::uint32_t* descriptor_worker_count,
    const DynEngineRuntimeContext&, DynProfileRootPartialNodeDelete delete_partial_00bf65ac);
} // namespace bsp
