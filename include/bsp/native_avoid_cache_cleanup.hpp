#pragma once

#include "bsp/cc13_avoid1.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native avoidance cache cleanup requires MSVC Win32.
#endif

namespace bsp {

static_assert(offsetof(Cc13AvoidZoneManagerNative, vector_first) == 0x60);
static_assert(offsetof(Cc13AvoidZoneManagerNative, vector_last) == 0x64);
static_assert(offsetof(Cc13AvoidZoneManagerNative, vector_end) == 0x68);
static_assert(offsetof(Cc13AvoidZoneManagerNative, list_head) == 0x70);
static_assert(offsetof(Cc13AvoidZoneManagerNative, list_size) == 0x74);
static_assert(offsetof(Cc13AvoidZoneHullListNode, value) == 8);

// Complete 0041E960..0041EA23 schedule over the existing 78h manager and real
// hull handle/node schemas. Names are hypotheses. Native ECX manager, no stack
// arguments, plain RET. New Source fastcall ABI: ECX manager; EDX the borrowed
// ADDRESS of the actual 00E188A8 publication cell. The current published game
// pointer is loaded at every queue site, followed by its +18h service pointer.
// No cached game pointer, substitute global or shadow publication is created.
//
// Concrete public queue implementation, matched singleton allocator/free and
// existing typed hull destroy are used; there are no abstract success bindings.
// The hull bridge supplies the genuine current heap pair through the existing
// AvoidZoneDynHullMemory contract. The typed destroy captures data once and
// reloads its fields after release. Current CRT/heap/handler/exception identity
// and Source context/provider frames remain qualified against the original.
//
// Body slots/header are reloaded while queueing. Returning invalid-parameter
// continuations retain cached old last, signed SAR counts and secure-copy guards.
// The sentinel is self-linked and size zeroed before node destruction; each next
// pointer is captured before destroying/freeing the node, and head is reloaded
// after each return. Vector capacity and sentinel remain allocated. Untouched
// manager fields are not reset. All reached backing/context/stack/control and
// provider state must remain valid; aliases retain the raw instruction schedule.
// No local EH cleanup/rollback or production/native ABI/gameplay proof is added.
// Evidence: docs/GOAL15C_AVOID_CACHE_CLEANUP.md.
void __fastcall clear_native_avoid_cache_0041e960(Cc13AvoidZoneManagerNative*,
    void* const* actual_00e188a8_publication_cell);

} // namespace bsp
