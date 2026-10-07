#pragma once

namespace bsp {

// Complete seven-byte leaves: native ECX is the same actual plane, EDX is
// unused, no stack arguments, RET, and one FLD result remains in x87 ST0.
// Callers must supply readable live binary32 storage at +C6C or +B1C, with
// its lifetime intact and at least one free x87 slot. No copied field, table,
// class admission or world service is supplied by these raw entry points.
// The ordinary fixture domain uses masked x87 exceptions. Faults, unmasked
// exceptions, original executable binding and gameplay remain unvalidated.
float __fastcall native_plane_heading_0074e260(
    const void* actual_plane, void* unused_edx);
float __fastcall native_plane_cached_speed_007b8e60(
    const void* actual_plane, void* unused_edx);

} // namespace bsp
