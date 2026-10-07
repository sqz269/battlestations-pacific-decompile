#pragma once
#include <cstdint>

namespace bsp {
struct NativeSessionMessageStorage;

// Complete 0076E470 ordinary body: ECX actual game+1EF0 session, one raw
// message pointer on the stack, RET4. This Source interface is not that ABI.
// Accesses actual +25C/+260/+264 without declaring a full session layout.
// Allocation uses the fixed malloc/new-handler service. Capacity is published
// before allocation; failure does not roll it back. No message ownership
// transfer, destructor dispatch, or downstream 0076C500 progress is inferred.
void append_native_session_message_0076e470(
    void* actual_session, NativeSessionMessageStorage* message);
} // namespace bsp
