#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission Lua machine storage requires MSVC Win32.
#endif

namespace bsp {

// Complete normal 006B8740..006B87B5 over actual writable 0Ch backing:
// DWORD+0 numerical profile CF8394, lua_State*+4, owns BYTE+8, padding+9..B.
// The caller supplies genuine live, aligned Win32 DWORD/pointer/byte storage.
// This function allocates no machine backing and authenticates no object.
// Caller supplies the genuine singleton_lifetime_allocate({object,12,12})
// allocation, retained for the matching singleton_lifetime_free domain below.
//
// Store CF8394, create the real linked Lua5.1.1 state and publish it at+4.
// The panic call receives that captured new state; each later API call reloads
// current state+4. Set GC pause(6,100), then open exactly base(""), table, io,
// os, string, math, debug: push real opener with zero upvalues, push row name,
// unprotected call(1,0). Write owns+8=1 ONLY after the seven calls return.
// Earlier owns preimage and padding remain untouched until those exact stores.
// No luaL_openlibs, package, null-result recovery, catch, close or rollback.
//
// The existing cc13_seg29::lua_machine_panic_006b8720 is reused as the linked
// library's C callback. It is not a copied Native ECX callback code address.
// Allocation/VM/callback metadata and all provider bindings stay independently
// live through each call and any partial failure. Unprotected Lua error/panic,
// nonlocal transfer and OOM retain the actual library policy. A foreign C++
// escape through Lua C frames does not establish safe reuse or automatic close.
// A failed constructor has NOT published a valid owns byte for cleanup.
//
// New ordinary C++ interface, not Native ECX/EDX, stack/flags/fault/FH3 ABI.
// Native numeric profile CF8394 is identity data, never callable Source code.
// This primitive does not supply 00884BE0 adoption or scalar/table transport.
void* construct_native_mission_lua_machine_006b8740(void* actual_machine);

// Complete normal 006B87C0..006B87EC. Capture owns BYTE+8, restore CF8394,
// then, iff captured owns!=0, reload state+4 and call real lua_close with NO
// state-null guard. After close returns, read only the low byte of this Source
// by-value flags word and free the captured machine base iff bit0 is set.
// Return that captured pointer even when retired; never read the receiver
// after free. No state/owns/padding clear and no repeated owns/state lookup.
//
// Caller retains the genuine allocation/domain named above. Captured nonzero
// owns additionally requires a valid current Lua state and independent live
// callback/services custody; owns0 does not read state+4. A numeric profile
// match proves none of those contracts. Flags1 uses that matching free domain.
// Flags0 STILL closes an owned VM and leaves stale fields; caller metadata must
// prevent replay. A failing/nonlocal close does not reach flag read or free.
// No implicit constructor-failure cleanup or foreign-exception recovery.
//
// The volatile Source argument-byte read pins the post-close epoch. It does
// not reproduce Native caller-stack aliases/placement, register or binary ABI.
// Full host/queue/payload retirement and authenticated CF8394/D0E7A8 routing
// remain separate contracts; no table/owner/callback projection is introduced.
void* delete_native_mission_lua_machine_006b87c0(
    void* actual_machine, std::uint32_t flags);

} // namespace bsp
