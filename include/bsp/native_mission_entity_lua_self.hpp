#pragma once

#include "bsp/native_lua_objects.hpp"

namespace bsp {

// Complete00927B40..00927BE1[162]. Native ECX=actual canonical entity,
// one fresh14h output pointer on stack, EAX=captured output, RET4. Source ABI
// borrows that entity's SAME NativeString at+178 and the actual E188A8 cell.
// No copied key, semantic-unit layout cast, private Lua state or world owner.
// World+1A0C must contain the existing, constructed NativeLuaStateStorage.
// Capture current world once; GetGlobals, GetByName("thisTable"), then read
// current key through GetByNativeString. Destroy thisTable then globals;
// the surviving output's tracked stack index is updated by actual cleanup.
// No null/closed world, nil-table or tracking-capacity fallback is invented.
NativeLuaObjectStorage* construct_native_mission_entity_lua_self_00927b40(
    const NativeString& actual_entity_key_178,
    NativeLuaObjectStorage& fresh_output,
    void* volatile& actual_world_publication_00e188a8);

// Existing actual Lua object/state/tracking providers and Lua5.1.1 are used
// directly. Three-state mapDDA2AC includes conditional completed-output
// cleanup after the two temporary objects. Source guards reproduce completed
// object effects for C++ unwinding; original FH3, mutable native stack aliases,
// Lua longjmp/error and hardware-fault cleanup remain unproved.
} // namespace bsp
