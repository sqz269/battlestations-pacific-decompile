#pragma once

#include "bsp/cc13_avoid1.hpp"

#include <cstdint>

namespace bsp {

// Borrow the two actual publication-cell ADDRESSES, with stable address
// bindings and context/slot lifetimes. Publication values may change. No
// replacement global, cached publication value or owner is made.
struct NativeAvoidManagerLifetimeContext {
    void* const* actual_00e188a8_publication_cell;
    Cc13AvoidZoneManagerNative** actual_00e17624_publication_cell;
};

// Complete standalone list clear at 0041E200. ECX points to the actual list
// header (+6Ch in the manager): proxy+0, head+4, size+8. Sentinel/proxy survive;
// embedded hulls and nodes use genuine current Source hull/free providers.
void __fastcall clear_native_avoid_hull_list_0041e200(void* actual_list_header);

// Raw 78h storage, not value-initialized storage or a C++ replacement class.
// Original ECX receiver/RET/EAX result are retained; EDX explicitly supplies
// Source context. Construction does not publish the manager. Unwind clears
// the actual E17624 cell even if it contains a different pointer, as observed.
Cc13AvoidZoneManagerNative* __fastcall construct_native_avoid_manager_00421500(
    Cc13AvoidZoneManagerNative* manager, const NativeAvoidManagerLifetimeContext* context);

// Complete normal operation and recovered list/vector/base unwind operations
// via current C++ guards, not transplanted Native FH3/SEH/frame machinery.
// Finishes with the numeric Native CE3818 base PROFILE word, not a callable
// Source base table. Flag-0 destruction leaves that raw destructed receiver.
void __fastcall destroy_native_avoid_manager_0041f980(
    Cc13AvoidZoneManagerNative* manager, const NativeAvoidManagerLifetimeContext* context);

Cc13AvoidZoneManagerNative* __fastcall scalar_delete_native_avoid_manager_00420680(
    Cc13AvoidZoneManagerNative* manager, const NativeAvoidManagerLifetimeContext* context,
    std::uint32_t flags);

using NativeAvoidManagerScalarDeleteEntry = Cc13AvoidZoneManagerNative* (__fastcall*)(
    Cc13AvoidZoneManagerNative*, const NativeAvoidManagerLifetimeContext*, std::uint32_t);
struct NativeAvoidManagerLifetimeSourceVtable {
    NativeAvoidManagerScalarDeleteEntry scalar_delete;
};
static_assert(sizeof(NativeAvoidManagerLifetimeSourceVtable) == 4);
extern const NativeAvoidManagerLifetimeSourceVtable kNativeAvoidManagerLifetimeSourceVtable;

// Source virtual callers must provide context in EDX (e.g. the typed table
// call above). Original CALL EDX loads the target into EDX and is insufficient.
// Only this consumed slot is provided; no Native RTTI/class/table identity or
// production publication/dispatch installation is asserted.

} // namespace bsp
