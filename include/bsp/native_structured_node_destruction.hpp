#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Complete BE9DF0 against an existing actual24h node. Native ECX=node, RET;
// this C++ interface adds the actual string-pool publication context.
// Stamp D68BB4; for an attached reader debit nonnull parent+20h by the FULL
// declared node+1Ch, decrement reader+60h and clear node+08h. Release the
// counted name at+10h through the existing raw pool, then stamp base CEB130.
// Base stamping also runs if name cleanup unwinds. No seek, remaining/count
// write, path-name destruction, allocation free or parent reference change.
// Requires valid native storage; no null/ownership/readiness guards are added.
// Evidence: docs/NATIVE_RAW_NODE_DESTRUCTION_BH.md. Compiler-generated cleanup
// preserves the schedule; native FH3 metadata and drop-in ABI are not claimed.
void destroy_native_structured_node_00be9df0(void* actual_node,
    NativeStringRawPoolContext& strings);
} // namespace bsp
