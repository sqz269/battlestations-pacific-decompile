#pragma once

namespace bsp {

// Complete raw entry 008849B0..008849CA: ECX receiver, no stack arguments,
// plain RET, EAX returns receiver. Caller supplies live writable 20-byte backing.
// Writes five DWORDs in order: 00D0E6F4h, FFFFFFFFh, 0, 0, 0. No null guard,
// allocation, prior-value reads or cleanup. No bytes within those 20 are retained;
// bytes outside that footprint are untouched by the five stores.
// The first word is an exact Native numerical profile identity; this leaf does
// not provide or validate a callable Source profile at that address. Descriptive
// name is provisional; no larger object layout or usable virtual owner is claimed.
void* __fastcall initialize_native_mission_lua_variant_008849b0(
    void* actual_element);

} // namespace bsp
