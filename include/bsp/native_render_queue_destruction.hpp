#pragma once

#include "bsp/native_render_queue_access.hpp"
#include "bsp/native_render_queue_execution.hpp"

namespace bsp {
class NativeStringStorage;

// Borrow the same actual queue publication, row string domain, execution
// providers and prepared persistent command frames as construction/submission.
// Null execution context is valid only for an initially nonpositive count,
// as in the full B1EBE0 provider. This binding owns no native storage.
struct NativeRenderQueueDestructionContext {
    NativeRenderCommandQueueStorage* volatile& actual_queue_00f8d440;
    NativeStringStorage& actual_row_strings;
    NativeRenderQueueExecutionContext* actual_execution;
    NativeRenderQueueExecutionFrame* actual_execution_frame;
};

// Complete B1F330..B1F3B9[138]: original ECX actual34h queue, RET. Restore
// D5E5F4, arm state2, execute the genuine B1EBE0 queue, then state1 rows
// resize0/current array free, state0 commands resize0/current array free,
// unconditionally clear F8D440 and restore CE3818. Context+30 is not released
// independently, and array data/capacity fields remain stale after freeing.
// C++ unwind follows the reviewed row/command/base states without retrying
// execution or independently destroying pointed commands.
void destroy_native_render_queue_00b1f330(NativeRenderCommandQueueStorage&,
    NativeRenderQueueDestructionContext&);

// Complete B1F6B0..B1F6CD[30]: original ECX queue, stack flags, EAX original
// owner, RET4. Full ordinary destructor, then free owner only when flags&1.
// An escaping ordinary-destructor exception does not free the owner here.
NativeRenderCommandQueueStorage* delete_native_render_queue_00b1f6b0(
    NativeRenderCommandQueueStorage&, std::uint32_t flags,
    NativeRenderQueueDestructionContext&);

// New source context/frame ABI, not original register/stack/FH3 metadata.
// C++ cleanup exceptions terminate; hardware-fault unwind, malformed storage,
// original CRT exception identity, concurrency and gameplay are not proved.
// No worker stop, manager unregister, extra context release or shutdown-only
// command substitute is added. Actual queue execution retains its full calls.
} // namespace bsp
