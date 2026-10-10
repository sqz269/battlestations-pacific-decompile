#pragma once

#include "bsp/native_string.hpp"

namespace bsp {

// Normalized payload cleanup for 00737CE0: native ECX=node, plain RET.
// The node has an eight-byte narrow header at +0 and wide header at +8.
// Keep the original node and supplied raw context readable/alive through both
// calls and any unwind cleanup; borrow the same canonical AA8/AA4/AA0 cells.
// Wide cleanup is guarded by CURRENT narrow-header cleanup on the original
// node. Disarm before normal narrow cleanup. Each helper captures its own
// buffer/size before its getter; headers are not cleared or deduplicated.
// Ordinary propagating-getter C++ exceptions are covered. The existing pool
// return remains noexcept; a cleanup exception during C++ unwind terminates.
// Native pointer-read/state fault timing, EH/SEH ABI and double-failure parity
// are not reproduced. No node-slot recycling, map or application activation.
// Evidence and boundaries: docs/CC12_LOCALE_MAP_RAW_NODE_PAYLOAD_HELPER.md.
void destroy_native_locale_map_node_payload_00737ce0(void* actual_node,
    NativeStringRawPoolContext& context);

} // namespace bsp
