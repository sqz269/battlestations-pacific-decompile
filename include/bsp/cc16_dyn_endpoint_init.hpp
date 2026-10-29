#pragma once

#include <cstdint>
#include "bsp/cc15_dyn_live.hpp"

namespace bsp::cc16_dyn_endpoint_init {

// 0040A050..0040A05D: EAX points to the actual vector's data word; RET.
// Frees its captured nonzero data through the required BF6989 provider.
// Leaves all vector fields unchanged. This is a new explicit C++ interface.
void release_vector_storage_0040a050(
    void* actual_vector, const cc15_dyn_live::DynLiveCrt& crt);

// 00C4CE60..00C4CF1A: ECX token, DL flag, stack(endpoint,bounds), RET8;
// EAX returns endpoint. Borrows the actual untyped endpoint and six-word bounds.
// Preserves interleaved bound copies, live allocator-visible vector fields and
// their publication order. No native class/vtable, fallback allocator or guard.
// C++ exceptions use the established vector cleanup then rethrow; original
// FH3/SEH, register ABI, fault PCs and exception transport are not supplied.
void* initialize_endpoint_00c4ce60(
    std::uint32_t token, std::uint8_t flag, void* endpoint, const void* bounds,
    const cc15_dyn_live::DynLiveCrt& crt);

}  // namespace bsp::cc16_dyn_endpoint_init
