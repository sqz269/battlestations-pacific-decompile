#pragma once

#include "bsp/native_hardware_layout_construct.hpp"

namespace bsp {

// Complete B2F710..B2F7F7. Native renderer ECX is unused; stack holds the
// actual stream-key/list address; EAX returns the selected owner; RET4.
// Borrow the constructor context's actual canonical tree, pool, owner/string
// domains and invalid-parameter service. Tree keys are raw stream identities.
//
// A hit validates the checked iterator, increments the actual owner's
// intrusive count, and returns it. A miss allocates a raw canonical slot and
// constructs it when nonnull. Only incomplete construction owns a raw-slot
// cleanup action. Disarm before pair construction/copy/insertion; a later
// failure does not destroy the completed owner or return its slot. Null raw
// allocation still inserts a null value. Ignore insertion's duplicate/result
// fields and return the newly constructed pointer without a cache AddRef.
void* get_or_create_native_hardware_layout_00b2f710(
    const void* actual_stream_key, NativeHardwareLayoutConstructContext&);

// New MSVC Win32 C++ interface; original calling convention is not exposed.
// The constructor's reached-input domain and existing tree/pool/invalid-handler
// contracts apply. The actual tree and pool identities remain fixed throughout
// this call. Every reached key/node/owner field must have valid backing storage;
// no key count clamp, null-hit repair, synthetic provider, ABI or game proof.

} // namespace bsp
