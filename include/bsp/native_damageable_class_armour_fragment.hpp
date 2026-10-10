#pragma once
#include "bsp/native_lua_objects.hpp"

namespace bsp {

// Borrow the enclosing 0087CA80 invocation's actual stable storage. S is ESP
// after its E4h locals and four saved registers, not this C++ function's frame.
// Unique at S+94h remains live in parent state1; HP at S+44h is dead. Stale
// scratch bytes are allowed. The enclosing owner retains and cleans Unique.
struct NativeDamageableClassArmourFragmentScratch {
    NativeLuaObjectStorage& live_unique_at_parent_94;
    void* fresh_field_at_parent_44; // aligned 14h bytes, stable address
};

// Armour only: 0087CCA8..0087CCE1; ExplosionType at successor CCE2 is excluded.
// CCA8 is an interior setup instruction, not an original entrypoint. Borrow the
// actual descriptor already held in native ESI, live row/current Lua owner,
// still-registered Unique and scratch. All regions must remain disjoint and
// stable, including the scratch pointer. The descriptor supplies writable raw
// bytes through +4Fh, aligned at least4; no C++ float subobject is required.
// Existing protected lookup owner/index stability, tracking-capacity and
// inherited error-handler-position contracts apply. No copied row is used.
//
// Stage the observed positive-zero fallback with FLDZ/FSTP before state7.
// Exact Lua NUMBER uses the existing x87 narrowing provider; numeric strings
// and other kinds retain the fallback. After an ordinary C++ float spill/reload,
// explicitly FSTP into actual raw descriptor+4Ch before state7->1 and actual
// tracked field destruction. Preserve earlier fields and Unique; their later
// lifetime and cleanup belong to the enclosing owner. Do not replay this field.
//
// This is strict MSVC Win32 source ABI with protected C++ Lua error transport.
// Extra C++ spills are not a native ABI bridge. Native stack/register/FH3,
// FP status/trap/SEH/longjmp and double-exception identity remain held.
// Secondary failure during guard unwinding follows ordinary noexcept termination.
void read_native_damageable_class_armour_fragment_0087cca8(
    void* actual_descriptor, NativeLuaObjectStorage& actual_row,
    NativeDamageableClassArmourFragmentScratch& actual_scratch);

} // namespace bsp
