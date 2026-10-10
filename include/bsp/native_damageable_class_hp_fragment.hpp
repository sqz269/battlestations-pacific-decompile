#pragma once
#include "bsp/native_lua_objects.hpp"

namespace bsp {

// Borrow the enclosing 0087CA80 invocation's actual stable storage. S is ESP
// after its E4h locals and four saved registers, not this C++ function's frame.
// Unique at S+94h is live in parent state1; the prior Comment field at S+44h is
// dead. Scratch bytes may be stale. The enclosing owner retains/cleans Unique.
struct NativeDamageableClassHpFragmentScratch {
    NativeLuaObjectStorage& live_unique_at_parent_94;
    void* fresh_field_at_parent_44; // aligned 14h bytes, stable address
};

// HP only: 0087CC66..0087CCA7, with Armour's successor CCA8 excluded. CC66 is
// an interior setup instruction, not an original entrypoint. Borrow the actual
// descriptor, live row/current Lua owner, still-registered Unique and scratch.
// These storage regions must not overlap; scratch address/pointer stays stable.
// The descriptor provides writable raw bytes through +4Bh, aligned at least 4;
// its HP storage need not contain an established C++ float subobject.
// Existing protected lookup owner/index stability, tracking-capacity and inherited
// error-handler-position contracts apply. No copied row, index or owner is used.
//
// Stage the retained 100.0f default through x87 before entering state6. Exact
// Lua NUMBER uses the existing x87 float32 narrowing provider; numeric strings
// and other kinds use the default. After an explicit C++ float spill/reload,
// store through x87 as float32 at actual descriptor+48h
// before lowering state6->1 and destroying the actual tracked HP field. Keep
// earlier descriptor writes and Unique alive; the enclosing parent owns later
// fields and cleanup on propagation. Do not replay a partial invocation.
//
// Explicit strict MSVC Win32 source ABI and existing protected C++ Lua error
// transport. Native stack/register/FH3, FP trap/SEH/longjmp and double-exception
// identity remain held. Existing provider ABI spills are not a binary bridge.
// Secondary failure during guard unwinding terminates under ordinary noexcept.
void read_native_damageable_class_hp_fragment_0087cc66(
    void* actual_descriptor, NativeLuaObjectStorage& actual_row,
    NativeDamageableClassHpFragmentScratch& actual_scratch);

} // namespace bsp
