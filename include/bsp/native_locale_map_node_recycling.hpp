#pragma once

#include "bsp/native_string.hpp"

#include <cstdint>

namespace bsp {

// 00736090: native ECX=pool, stacked DWORD slot, RET4. Borrow the actual
// initialized locale pool (E1768C), its live critical section at +0C and a
// genuinely allocated 1Ch slot belonging to its current block/table. A pointer
// or zeroed layout is not owner construction or canonical-authority evidence.
// Keep the provider's own recursive Enter/depth/Leave, even under a map lock.
// Read slot identity/table only after Enter and current-depth increment. The
// free-stack write uses captured WORD count; the subsequent increment rereads
// CURRENT count. No bounds/null checks, payload cleanup, rollback or free.
void return_native_locale_node_slot_00736090(
    void* actual_initialized_locale_pool, void* actual_allocated_node_slot);

// 0073C220: native ECX=node, stacked DWORD flags, EAX=original node bits, RET4.
// Reuse 00737CE0 payload cleanup with the same actual AA8/AA4/AA0 publication
// cells. Only after normal payload return, read flags' low byte and return the
// slot if bit0 is set; ignore other bits. The pool is accessed only on that
// path, and must be the same actual initialized E1768C authority. No early
// slot/table read, node free or local unwind cleanup. Payload failure prevents
// slot return. The result is address bits, not a live-object guarantee.
void* scalar_delete_native_locale_map_node_0073c220(void* actual_node,
    std::uint32_t flags, NativeStringRawPoolContext& actual_strings,
    void* actual_initialized_locale_pool_00e1768c);

// Dormant normalized MSVC Win32 interfaces, not original entry/stack ABI.
// Stable Enter/Leave import bindings are required when factoring a caller that
// caches Enter. Ordinary C++ payload exceptions retain the existing helper's
// limits; native fault/SEH timing, FH3 and double-failure parity are unproved.
// No owner, slab producer, trim, startup, registry or application is activated.
// Evidence: docs/CC12_LOCALE_BORROWED_NODE_RECYCLING_HELPER.md.

} // namespace bsp
