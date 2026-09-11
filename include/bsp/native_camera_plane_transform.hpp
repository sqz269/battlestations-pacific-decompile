#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera plane transforms require MSVC Win32 x87 and MOVSS.
#endif

namespace bsp {

// Full B62D10..B62DC8. Original ECX source, stack destination/matrix,
// EAX destination, RET8. New fastcall: ECX source, EDX destination,
// stack matrix, EAX destination, RET4. All pointers refer to raw storage.
// Snapshot source with ordered x87 float stores (Y,X,Z,W), then preserve
// the complete extended-intermediate schedule and four final float stores.
// Destination/source overlap uses the snapshot. Destination/matrix overlap
// can change later matrix reads after earlier output components are stored.
void* __fastcall transform_native_vector4_00b62d10(
    const void* actual_source_float4, void* actual_destination_float4,
    const void* actual_matrix_float16);

// Full B65BA0..B65C21. Same original/new calling interfaces as above.
// Snapshot source bits with MOVSS, call full B62D10 into a distinct local
// result, then copy all result components to the caller's destination.
// Matrix/output aliasing cannot feed final writes back into the calculation.
void* __fastcall transform_native_plane_00b65ba0(
    const void* actual_source_float4, void* actual_destination_float4,
    const void* actual_matrix_float16);

// No normalization, inverse/transpose construction, owner, callback,
// validation, floating-control policy or exception rollback is added.
// These raw interfaces are not asserted to replace original native callers.

} // namespace bsp
