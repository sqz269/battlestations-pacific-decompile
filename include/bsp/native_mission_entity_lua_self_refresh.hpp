#pragma once

#include "bsp/native_lua_objects.hpp"

namespace bsp {

// 00928B73..00928B9A [40 bytes / 11 operations], inside 00928A00.
// After fresh-table creation and its temporary cleanup, reacquire self,
// assign into the persistent object, then destroy the new temporary BEFORE ID.
// This is an ordinary C++ fragment interface, not a Native ABI entry.
//
// Borrow the SAME actual R+178 NativeString and current E188A8 cell. The
// persistent 14h self object is already constructed at a stable address.
// The function supplies distinct fresh 14h temporary storage. Actual old/new
// owners, states, key storage, tracking capacity and lifetimes must remain
// valid through Lua callbacks, destination release and temporary destruction.
// The providers perform all current owner/index/count reads and registration;
// callers must use persistent_self's CURRENT fields after this returns.
//
// Getter failure inherits that provider's completed-output cleanup. After
// getter success, a C++ assignment exception causes one temporary-destruction
// attempt, then is rethrown if cleanup returns. If cleanup itself throws, its
// exception replaces the original C++ assignment exception. Cleanup is never
// retried; no consumed flag is invented. Effects survive failure without rollback.
// Original saved-stack/FH3, Lua nonlocal errors, faults and arbitrary aliases
// are not established. This does not implement the whole attach or own R.
void refresh_native_mission_entity_lua_self_00928b73_fragment(
    const NativeString& actual_entity_key_178,
    NativeLuaObjectStorage& persistent_self,
    void* volatile& actual_world_publication_00e188a8);

} // namespace bsp
