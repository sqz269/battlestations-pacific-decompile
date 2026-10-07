#pragma once

#include <cstdint>

namespace bsp {
class NativeObserverLifetime;

// Complete normal 0070D260..0070D283, CFD6F8 slot zero. Native ECX=the
// actual 508h group, stack=flags, EAX=original address, RET4. Stamp CFD6F8,
// run the existing whole callback-owner destructor 00695870, then free the
// group through the existing CRT boundary iff flags&1. Capture the address
// before deletion; the returned value is opaque when that low bit is set.
// Storage must come from the matching allocation domain, have the actual
// observer-owner prefix, and belong to the supplied live observer runtime.
// No null guard or repeated destruction is supported. This new C++ interface
// does not provide the native class ABI, callable profile, old CRT heap,
// exception ABI, group publisher, or world lifetime.
void* delete_native_unit_group_0070d260(
    void* group, std::uint32_t flags, NativeObserverLifetime&);
} // namespace bsp
