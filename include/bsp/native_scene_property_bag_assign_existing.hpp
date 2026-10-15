#pragma once

#include "bsp/native_scene_property_insertion.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Scene property AssignExistingFrom requires MSVC Win32.
#endif

namespace bsp {

// Whole ordinary successful-domain [008F23E0,008F253F),351B/118 instructions.
// Descriptive name is a hypothesis. Borrow actual raw61-produced destination
// and source bags, nonnull and finite, with live unique consistent map chains.
// Both owning graphs/payloads are disjoint, unshared and acyclic. Complete
// CC8A30 startup; borrow the SAME canonical string/property-pool generation
// through this concrete context. Closed NUL-free ASCII keys/current C locale,
// stable live storage, DF0, sufficient stack and successful normal providers
// are required; no concurrent mutation, trim, reentry, overflow or invalid alias.
//
// Allocate/begin/free a real12B iterator even for an empty source. Each actual
// raw8B query uses the current pooled constructor/lookup/return. Capture the
// destination mapped word BEFORE retiring the query, then decide hit/miss.
// A miss skips WITHOUT reading a source mapped record or changing topology.
// On a hit physically read DESTINATION tag FIRST. Tag6 then reloads source
// iterator/node/record/child and recurses on the existing child bags. Other
// tags then reload the source record and invoke full008F0700 assignment.
//
// Matching hit tags and compatible Type6 children are input preconditions,
// not added checks. Inherit the complete assignment's owning-text/reference,
// positive-byte array/flag1 and accepted Type1 FP contracts. Source remains
// borrowed and unchanged. No self/ancestor/shared-old-buffer safety shortcut.
// No clone, insertion, keep flag, new fallback cell, model or host callback.
// Bag count/head/key/node/counter/owner topology and record metadata remain
// destination values; only admitted existing payloads/descendants are assigned.
// Retire owned records/keys/nodes before the canonical pools are torn down.
//
// New void C++ ABI only. No Original ECX/RET4/residual-register/flags/FS/SEH/EH,
// OOM, historical private CRT, class/vtable, application or game qualification.
void assign_existing_native_scene_property_bag_008f23e0(
    void* actual_destination_bag, const void* actual_source_bag,
    NativeScenePropertyInsertionContext& context);

} // namespace bsp
