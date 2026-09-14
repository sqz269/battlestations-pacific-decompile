#pragma once

namespace bsp {

// Complete raw 00B6E0D0 getter over the SAME actual native node storage used
// by B6DB70. Original ECX is the node; EAX returns its +60h inverse-world
// cache address, with plain RET and no ownership operation. Flags are the
// current DWORD at +5Ch; world matrix is the current 64 bytes at +F0h.
// A cache hit returns +60h directly. On a miss, the original entry flag DWORD
// determines whether to refresh world first; inverse/copy use real raw matrix
// providers and the cache-valid bit is ORed into CURRENT flags only afterward.
// No null, hierarchy, singular-matrix, alias, or native exception repair.
void* __fastcall get_native_node_inverse_world_00b6e0d0(void* actual_node);

// This is a raw source interface, distinct from the existing typed
// get_transform_inverse_world_00b6e0d0(CameraTransform&) projection.

} // namespace bsp
