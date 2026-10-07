#pragma once

#include "bsp/native_land_moveto_constructor.hpp"

namespace bsp {

// COMPLETE009C3D10..009C3D2E30B/11instructions, new SOURCE interface.
// Preserve existing CG_scalar_deleting_dtor_009c3d10 library role/name/tag.
// Original ECX=actual MoveTo ROOT, stacked DWORDflags, EAX=original ROOT,
// RET4. Captured ROOT ->complete007B65E0 ->LOWbytebit0 test ->optional genuine
// current CRT ROOTfree ->captured ROOT return. The test is AFTER the helper.
// Numeric identity return after free never permits dereferencing the receiver.
void* scalar_delete_native_land_moveto_009c3d10(
    const NativeLandMoveToConstructorView&, std::uint32_t flags,
    NativeObserverLifetime&);

// COMPLETE009C2B70..009C2B788B/2instructions: SUB ECX,18h; JMP009C3D10.
// D20AD4 slot0 is an ADJUSTOR, not a standalone callback scalar destructor.
// PURE address-only SOURCE admission verifies incoming actual ROOT+18 against
// the existing same-root view, then routes to the COMPLETE ROOT scalar above.
// Both flags0/1 return ROOT identity, not the incoming callback identity.
void* scalar_delete_native_land_moveto_callback_adjustor_009c2b70(
    const NativeLandMoveToConstructorView&, NativeObserverOwnerStorage* callback,
    std::uint32_t flags, NativeObserverLifetime&);

// Mandatory ordinary domain inherited from the actual >=3Ch constructor view:
// stable same-root backing, actual embedded callback18/vectorC, live coherent
// endpoint/registration and genuine NativeObserverLifetime with actual current
// manager/lock/dispatch/CRT context. flags1 requires a SEPARATELY allocated
// complete actual CRT MoveTo ROOT, never task+4C4/approach+CC/interior arena,
// callback+18, vector element or standalone callback allocation. No field/view
// dereference after ROOTfree or freed-array dereference after member cleanup.
// Raw D20AEC/D20AD4 are UNCALLABLE; no executable class profile/arena/default
// world or type-aware callback is supplied. Structural reentry/concurrency,
// invalid storage/allocation/fault/privateEH, full class ABI and game lifetime
// remain excluded. Invalid adjustor aliases report SOURCE admission errors,
// not a recovered native guard or fallback. Native encodings are not retained.

} // namespace bsp
