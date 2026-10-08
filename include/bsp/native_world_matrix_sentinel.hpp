#pragma once

namespace bsp {
// Complete 004C3080 allocation leaf. Incoming Native ECX is unused.
// Returns owned raw 0x6C-byte storage with self-links only at +0 and +4.
// Caller owns the result in the singleton_lifetime_allocate/free domain.
// Throws through the established host CRT allocation boundary.
void* __cdecl create_native_world_matrix_sentinel_004c3080();
} // namespace bsp
