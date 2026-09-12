#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native singleton publication requires MSVC Win32.
#endif

namespace bsp {

// Complete source behavior of 00415350[105]. Native entry consumes no input,
// returns EAX, plain RET. This source interface adds a stable reference to the
// actual mutable 01090AA0 publication cell. Capture its first read; if absent,
// allocate raw14h, construct through BD0960, publish and return the result.
// There is no publication lock or final reload. Constructor failure frees the
// captured allocation after the constructor's own cleanup, then rethrows.
void* get_native_singleton_manager_00415350(
    void* volatile& actual_manager_publication_01090aa0);

// Complete source behavior of 00B1B730[200]. Native entry consumes no input,
// returns EAX, plain RET. Borrow distinct actual native publication cells;
// these bindings remain stable while their volatile values may change.
// Capture the first manager's raw section+10, enter/increment once, recheck
// the registry, allocate raw10h and call B1AA70 before storing D5E59C. Publish,
// look up the manager again, then reread the registry and register through
// BD0C30. Keep the original captured section through release; the slow return
// rereads the registry after LeaveCriticalSection. The fast return is captured.
// Constructor failure frees its raw allocation. Registration failure retains
// the publication/allocation and invokes full 411EE0 on the captured guard.
void* get_native_resource_registry_00b1b730(
    void* volatile& actual_manager_publication_01090aa0,
    void* volatile& actual_registry_publication_00f8d41c);

// Actual source CRT allocation/free, raw constructors/registration, and Win32
// critical-section providers are required. No private global, callback shim,
// typed SingletonLifetimeDomain projection or new destructor policy is added.
// Original FH3/SEH stack maps, mutable EH-spill aliases, hardware-fault cleanup,
// provider register/throw identities and the original no-input callable ABI
// are not reproduced. Native profiles are identity DWORDs, not callable C++
// vtables. Raw mixed-owner destruction and canonical executable migration are
// separate unresolved contracts; these entries are not game-validated.

} // namespace bsp
