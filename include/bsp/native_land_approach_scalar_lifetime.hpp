#pragma once

#include "bsp/native_land_approach_lifetime.hpp"

namespace bsp {

// COMPLETE009B3220..009B323E: 30B/11 instructions. Preserve the existing
// CG_scalar_deleting_dtor_009b3220 library name/tag. Original ECX=WHOLE approach
// ROOT, stacked DWORDflags, EAX=captured ROOT, RET4. Capture ->complete009B2C80
// ->LOW byte bit0 test AFTER cleanup ->optional genuine current CRT ROOTfree
// ->captured ROOT identity. This is a new SOURCE interface, not a class ABI
// bridge. Returning the numeric identity after free permits no dereference.
void* scalar_delete_native_land_approach_009b3220(
    const NativeLandApproachCleanupView&, std::uint32_t flags,
    NativeObserverLifetime&);

// COMPLETE009B3030..009B303B: 11B/2 instructions, SUB ECX,B8; JMP009B3220.
// Actual inner registry profile D1FF84 slot0 selects this adjustor. Inner ROOT
// profile D1FF88 slot0 is009AFAD0; its +4 is009B3220. Neither raw image profile
// is callable here. D1FF90/D1FF94 describe a separate outer/task profile graph.
// PURE address-only admission validates incoming actual ROOT+B8 and the same
// view's registry identity, then invokes the COMPLETE ROOT scalar above.
// Return and optional free use ROOT, never the registry receiver. This is NOT
// the standalone registry411810 scalar contract. A mismatch reports a SOURCE
// admission error, not a recovered native guard or fallback.
void* scalar_delete_native_land_approach_from_registry_009b3030(
    const NativeLandApproachCleanupView&, NativeBotStateRegistryStorage* registry,
    std::uint32_t flags, NativeObserverLifetime&);

// Require the complete ordinary helper's coherent stable same-root >=26Ch
// borrowed storage, genuine observer/manager/publication/recursive-lock context,
// known24B element profile and successful current same-CRT operations. Cleanup
// extent26Ch is NOT recovered class sizeof: inner constructor reaches275h and
// is not supplied here. flags1 requires an independently allocated COMPLETE
// same-CRT WHOLE root, never task+3F8, registry+B8, embedded state/callback,
// interior arena or standalone registry. No post-free receiver/array reads or
// repeated cleanup. No default profile/world, constructor/arena, ownership or
// lifetime extension is supplied. Structural reentry, invalid storage/free,
// failures/overflow/fault/privateEH, native class ABI and gameplay remain
// outside admission. Raw image profiles remain UNCALLABLE data.

} // namespace bsp
