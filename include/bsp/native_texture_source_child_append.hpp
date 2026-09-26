#pragma once

#include "bsp/native_texture_source_storage.hpp"

namespace bsp {

// Source leaf for C30570's C307AD..C307E1 append fragment, not a complete
// native function or original ABI replacement. Caller reaches this after
// capturing EDI's actual renderer result and returning the temporary string.
// Payload/header must be the live C30470-created subobjects. Current backing
// and reached source slots must satisfy the genuine typed735FF0 contracts.
// Reentrant allocation callbacks may mutate fields but retain these lifetimes.
// captured_child may be null: store its exact pointer value at each nonnull
// placement address, then increment CURRENT count even if placement is skipped.
// Begin one scalar pointer lifetime only at that reached store; no new payload,
// header/atomic construction, retain/release, default data or rollback occurs.
// Native null-child retirement is not made safe. Caller keeps any returned-but-
// unappended resource obligations on failure; this leaf owns no child credit.
void append_native_texture_source_child_00c307ad_fragment(
    NativeTextureSourcePayload& payload, void* captured_child);

} // namespace bsp
