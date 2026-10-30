#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission Lua registration requires MSVC Win32.
#endif

struct lua_State;

namespace bsp {

using NativeMissionLuaCFunction = int (__cdecl*)(::lua_State*);

// Complete normal 006B8610..006B8636 over actual raw mission-machine backing.
// Original ECX=machine, stack(name, function), RET8. This ordinary interface
// does not reproduce its registers, caller-stack aliases, faults or binary ABI.
//
// Capture function, then CURRENT machine state+4; real lua_pushcclosure uses
// ZERO upvalues. After it returns, read the Source by-value name argument and
// reload CURRENT state+4 for real lua_setfield(L, LUA_GLOBALSINDEX, name).
// The two reached states need not be the same. No state/profile/owns/name or
// callback validation, host projection, catch, stack restoration or rollback.
// A second-call escape retains the first call's effects, including its closure.
// The volatile parameter-byte read pins this Source epoch only, not the Native
// caller's physical stack or its aliasing with reached machine/Lua storage.
//
// Caller supplies a genuine executable linked Lua5.1 C callback with the stated
// cdecl signature; an Original numeric address is not such a Source binding.
// Retain actual machine+4, each reached state/stack, NUL-terminated name, callback
// code and all callback/allocator/service dependencies through the operation
// and later registered use. Exact Lua/nonlocal/OOM/panic and current-provider
// behavior remains inherited; normal return alone authenticates no graph or
// completed factory. This leaf owns no machine, VM, name or temporary cleanup.
// No table iteration, two-upvalue semantic trampoline, Native handler mapping,
// embedded-owner adoption, Game readiness or startup activation is supplied.
void register_native_mission_lua_global_006b8610(void* actual_machine,
    const char* name, NativeMissionLuaCFunction function);

} // namespace bsp
