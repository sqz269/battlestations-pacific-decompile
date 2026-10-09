#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native pending registry construction requires MSVC Win32.
#endif

namespace bsp {

// Ordinary 00874BC0[69]: stamp D0DEA0, create the actual BD1860 section,
// then store its returned pointer at +04 and return the captured raw8 receiver.
// The borrowed publication cell is used only by Source C++ failure cleanup:
// clear that current cell and reset the base profile before rethrowing.
// This explicit-reference interface is not the original ECX/FH3/SEH entry.
// Numeric profiles do not supply callable Source tables or owner lifetime.
void* construct_native_pending_registry_00874bc0(
    void* actual_receiver,
    void* volatile& actual_registry_publication_00f878cc);

} // namespace bsp
