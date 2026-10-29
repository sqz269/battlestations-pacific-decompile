#pragma once

#include "bsp/cc15_dyn_live.hpp"

namespace bsp::cc16_dyn_endpoint_allocate {

// Complete normal 00C55A60..00C55D26. Native ECX manager; stack token,
// six-word bounds pointer, third-slot low-byte flag; RET0Ch; EAX endpoint.
// This new cdecl C++ interface borrows actual raw manager storage and the same
// real CRT used by the concrete endpoint initializer. Required CRT slots are
// BF9F1A malloc, BF55BE new and BF6989 free; no default/guard is provided.
//
// Both flag-selected 68000-byte/1000-slot pools, intrusive links, page vectors,
// pending vector and all allocator-visible count/data reloads remain. Objects
// stay untyped. It composes the real cc16_dyn_endpoint_init::initialize_endpoint
// implementation and ignores that call's return, retaining its captured endpoint.
// Native C55A60's state0 unwind action calls a real RET-only function; no outer
// cleanup/rollback is fabricated. The initializer retains its own real cleanup.
// Original FH3/SEH, frame/fault identity, concurrency and ABI transport are not
// supplied by this interface. Allocation failure can leave partial mutation.
void* __cdecl allocate_endpoint_00c55a60(
    void* manager, std::uint32_t token, const void* bounds,
    std::uint8_t flag, const cc15_dyn_live::DynLiveCrt& crt);

}  // namespace bsp::cc16_dyn_endpoint_allocate
