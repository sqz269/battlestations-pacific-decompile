#pragma once
#include "bsp/native_lua_objects.hpp"

namespace bsp {

// Borrow the actual enclosing 0087CA80 storage. S is ESP after E4h locals and
// four saved registers. Unique at S+94h is still live under parent state1;
// Armour at S+44h is dead. The enclosing owner retains and cleans Unique.
struct NativeDamageableClassExplosionTypeFragmentScratch {
    NativeLuaObjectStorage& live_unique_at_parent_94;
    void* fresh_field_at_parent_44; // aligned 14h bytes; stale bytes allowed
};

// ExplosionType only: 0087CCE2..0087CD4C; successor CD4D is excluded. CCE2 is
// an interior setup instruction, not a native entrypoint. Borrow the actual
// descriptor (already in native ESI), row/current Lua owner, Unique and scratch.
// Storage regions and scratch pointer remain stable and disjoint. Descriptor
// raw bytes through +43h are writable, aligned at least4; no int32 subobject
// lifetime is assumed. Existing protected lookup owner/index, tracking-capacity
// and inherited error-handler-position contracts apply across both lookups.
//
// First lookup: state8, exact existing integer-number predicate, then state1
// and destruction BEFORE testing the saved result. False leaves +40h untouched.
// True performs a FRESH lookup under state9 and converts that new value without
// another predicate. Publish its int32 bits at raw descriptor+40h, then lower
// to state1 and destroy the actual second field. Keep earlier writes and Unique.
// No enum mapping, default value, string acquisition or cached first value.
//
// The required CRT mode is the existing live Source bool decision corresponding
// to 0109EEA4; its genuine object must remain alive and must not overlap the
// other borrowed storage. Pass its alias, never a default, temporary or snapshot.
// It may change during Lua callbacks; the existing converter reads it only after
// the SECOND Lua numeric read and float32 narrowing. This does not reinterpret
// the native DWORD as bool or supply a native-global binding adapter.
//
// Ordinary strict MSVC Win32 Source and protected C++ Lua error transport only.
// Extra ABI spills, native DWORD binding, stack/register/FH3, FP traps/status,
// SEH/longjmp and double-exception identity remain held. Guard secondary failure
// follows ordinary noexcept termination. Do not replay a partial invocation.
void read_native_damageable_class_explosion_type_fragment_0087cce2(
    void* actual_descriptor, NativeLuaObjectStorage& actual_row,
    NativeDamageableClassExplosionTypeFragmentScratch& actual_scratch,
    const bool& actual_crt_sse2_conversion);

} // namespace bsp
