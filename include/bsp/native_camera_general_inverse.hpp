#pragma once

namespace bsp {
// Complete B632D0: ECX destination, EDX source, EAX destination, plain RET.
// Raw 64-byte matrix views; snapshots the source with REP MOVSD before
// publishing the destination identity and performing the original pivoting.
// Native x87/SSE state, source/destination aliasing and singular behavior
// are retained. There is no singularity check or mathematical substitution.
// Reconstructed immutable -0/+1 words match D7A208/D7A24C in this target.
void* __fastcall invert_native_camera_matrix_00b632d0(
    void* destination, const void* source);
} // namespace bsp
